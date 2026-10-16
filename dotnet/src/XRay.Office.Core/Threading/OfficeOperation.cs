using XRay.Office.Core.Documents;

namespace XRay.Office.Core.Threading;

/// <summary>
/// The cancellation and time budget of the operation running on the current thread.
/// </summary>
/// <remarks>
/// <para>
/// <b>Why ambient.</b> Loading, laying out and rendering a document is hundreds of methods deep, and
/// the loops that bound latency (a paragraph, a table row, a slide's shapes, a formula pass, a part of
/// a package) are scattered across all of them. Threading a token through every signature would touch
/// most of the code base and change nothing about how the loops behave, so the token travels with the
/// operation instead: a public entry point (<c>OfficeDocument.Open</c>, <c>IPaginatedDocument.Layout</c>,
/// <c>PdfRenderer.Render</c>, ...) opens a scope with <see cref="Begin"/> or <see cref="Run{T}(Func{T}, CancellationToken)"/>, and a
/// loop calls <see cref="Poll"/> or <see cref="ThrowIfCancelled"/>.
/// </para>
/// <para>
/// The scope is per thread. Everything here is synchronous, so an operation never leaves its thread;
/// code that is not inside a scope pays one thread-static read and a null test, which is why the
/// checkpoints can sit in loops without a flag to turn them off.
/// </para>
/// <para>
/// <b>A timeout is not a caller's cancellation.</b> When the scope has a time limit and it is what
/// fired, the operation throws <see cref="LayoutTimeoutException"/>; when the caller's token fired, it
/// throws a plain <see cref="OperationCanceledException"/> carrying that token. Both derive from
/// <see cref="OperationCanceledException"/>, so a caller that does not care can catch one type.
/// </para>
/// </remarks>
public static class OfficeOperation
{
    /// <summary>How many <see cref="Poll"/> calls pass between two real checks.</summary>
    private const int PollInterval = 32;

    [ThreadStatic]
    private static Scope? s_current;

    /// <summary>True when the current thread is inside an operation that can be cancelled.</summary>
    public static bool IsCancellable => s_current?.Cancellable == true;

    /// <summary>The token of the operation running on this thread, or <c>default</c> outside one.</summary>
    public static CancellationToken Token => s_current?.Token ?? default;

    /// <summary>
    /// Opens a scope on the current thread. Dispose it to leave the scope.
    /// </summary>
    /// <param name="timeout">
    /// A time limit for the work done inside the scope, or null for none. A zero or negative limit
    /// is treated as "no limit", the way a zero <c>MaxPages</c> is.
    /// </param>
    /// <param name="layout">The layout options whose picture limits apply inside the scope.</param>
    /// <param name="cancellationToken">The caller's token.</param>
    /// <remarks>
    /// Scopes nest: an inner scope is cancelled when either its own token or the enclosing scope is.
    /// </remarks>
    public static IDisposable Begin(
        TimeSpan? timeout = null, LayoutOptions? layout = null, CancellationToken cancellationToken = default)
    {
        Scope? outer = s_current;
        TimeSpan? limit = timeout is { } t && t > TimeSpan.Zero ? t : null;
        bool outerWatches = outer?.Cancellable == true;

        CancellationTokenSource? linked = null;
        CancellationToken token = cancellationToken;

        if (limit is not null || outerWatches)
        {
            if (outerWatches && cancellationToken.CanBeCanceled)
                linked = CancellationTokenSource.CreateLinkedTokenSource(outer!.Token, cancellationToken);
            else if (outerWatches)
                linked = CancellationTokenSource.CreateLinkedTokenSource(outer!.Token);
            else
                linked = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);

            if (limit is { } span) linked.CancelAfter(span);
            token = linked.Token;
        }

        Scope scope = new()
        {
            Source = linked,
            Token = token,
            Cancellable = token.CanBeCanceled,
            Outer = outer,
            Limit = limit,
            CallerToken = cancellationToken,
            Layout = layout ?? outer?.Layout,
        };
        s_current = scope;
        return new Handle(scope, outer);
    }

    /// <summary>The picture size limit of the layout running on this thread; zero for none.</summary>
    public static long MaxImagePixels => s_current?.Layout?.MaxImagePixels ?? 0;

    /// <summary>What happens to a picture over <see cref="MaxImagePixels"/> in the running layout.</summary>
    public static OversizedImagePolicy OversizedImages => s_current?.Layout?.OversizedImages ?? OversizedImagePolicy.Skip;

    /// <summary>
    /// Runs a layout: a scope with the options' time limit and picture limits, around
    /// <paramref name="work"/>.
    /// </summary>
    public static T Layout<T>(LayoutOptions? options, Func<T> work, CancellationToken cancellationToken = default)
        => Run(options?.Timeout, options, work, cancellationToken);

    /// <summary>
    /// Runs <paramref name="work"/> inside a scope and translates a timeout into
    /// <see cref="LayoutTimeoutException"/>.
    /// </summary>
    public static T Run<T>(
        TimeSpan? timeout, LayoutOptions? layout, Func<T> work, CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(work);
        cancellationToken.ThrowIfCancellationRequested();

        using IDisposable handle = Begin(timeout, layout, cancellationToken);
        Scope? scope = s_current;

        try
        {
            return work();
        }
        catch (OperationCanceledException ex) when (ex is not LayoutTimeoutException
            && scope is { Limit: not null }
            && !scope.CallerToken.IsCancellationRequested
            && scope.Token.IsCancellationRequested
            && scope.Outer?.Token.IsCancellationRequested != true)
        {
            throw new LayoutTimeoutException(scope.Limit.Value, ex);
        }
    }

    /// <summary>Runs <paramref name="work"/> inside a scope with no time limit.</summary>
    public static T Run<T>(Func<T> work, CancellationToken cancellationToken = default)
        => Run(null, null, work, cancellationToken);

    /// <summary>Runs <paramref name="work"/> inside a scope; see <see cref="Run{T}(TimeSpan?, LayoutOptions?, Func{T}, CancellationToken)"/>.</summary>
    public static void Run(Action work, CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(work);
        Run<object?>(() =>
        {
            work();
            return null;
        }, cancellationToken);
    }

    /// <summary>
    /// Throws if the current operation has been cancelled or has run out of time.
    /// </summary>
    /// <remarks>A thread-static read, a null test and a flag read: cheap enough for a per-item loop.</remarks>
    public static void ThrowIfCancelled()
    {
        Scope? scope = s_current;
        if (scope is null || !scope.Cancellable) return;
        if (scope.Token.IsCancellationRequested) throw scope.Exception();
    }

    /// <summary>
    /// A checkpoint for a hot loop: checks for real once every few calls.
    /// </summary>
    public static void Poll()
    {
        Scope? scope = s_current;
        if (scope is null || !scope.Cancellable) return;
        if ((++scope.Ticks & (PollInterval - 1)) != 0) return;
        if (scope.Token.IsCancellationRequested) throw scope.Exception();
    }

    private sealed class Scope
    {
        public CancellationTokenSource? Source { get; init; }
        public CancellationToken Token { get; init; }
        public bool Cancellable { get; init; }
        public Scope? Outer { get; init; }
        public TimeSpan? Limit { get; init; }
        public CancellationToken CallerToken { get; init; }
        public LayoutOptions? Layout { get; init; }
        public int Ticks;

        public OperationCanceledException Exception()
        {
            // The caller's own cancellation wins over a timeout that happened to fire at the same time,
            // and an enclosing scope's reason is passed on rather than relabelled.
            if (CallerToken.IsCancellationRequested) return new OperationCanceledException(CallerToken);
            if (Outer is { Cancellable: true } outer && outer.Token.IsCancellationRequested) return outer.Exception();
            if (Limit is { } limit) return new LayoutTimeoutException(limit);
            return new OperationCanceledException(Token);
        }
    }

    private sealed class Handle(Scope scope, Scope? outer) : IDisposable
    {
        private bool _disposed;

        public void Dispose()
        {
            if (_disposed) return;
            _disposed = true;
            s_current = outer;
            scope.Source?.Dispose();
        }
    }
}

/// <summary>
/// An operation ran past its <c>Timeout</c>.
/// </summary>
/// <remarks>
/// Derives from <see cref="OperationCanceledException"/> so that code written for cancellation handles
/// it, and is a separate type so that a caller can tell "I cancelled" from "the document took too long".
/// </remarks>
public sealed class LayoutTimeoutException : OperationCanceledException
{
    /// <summary>Creates the exception.</summary>
    /// <param name="timeout">The limit that was exceeded.</param>
    public LayoutTimeoutException(TimeSpan timeout)
        : base($"The operation exceeded its time limit of {timeout}.")
        => Timeout = timeout;

    /// <summary>Creates the exception with the cancellation that carried it.</summary>
    public LayoutTimeoutException(TimeSpan timeout, Exception? innerException)
        : base($"The operation exceeded its time limit of {timeout}.", innerException)
        => Timeout = timeout;

    /// <summary>The limit that was exceeded.</summary>
    public TimeSpan Timeout { get; }
}
