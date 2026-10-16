using XRay.Office.Core.Documents;
using XRay.Office.Core.Threading;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// The ambient scope every cancellable operation runs in, and the stream that makes a parser's reads
/// checkpoints.
/// </summary>
public sealed class OfficeOperationTests
{
    [Fact]
    public void OutsideAScopeACheckpointCostsNothingAndNeverThrows()
    {
        OfficeOperation.IsCancellable.ShouldBeFalse();
        OfficeOperation.Token.CanBeCanceled.ShouldBeFalse();
        for (int i = 0; i < 1000; i++) OfficeOperation.Poll();
        OfficeOperation.ThrowIfCancelled();
    }

    [Fact]
    public void ACancelledTokenIsNoticedByAPollWithinAFewCalls()
    {
        using CancellationTokenSource source = new();
        using (OfficeOperation.Begin(cancellationToken: source.Token))
        {
            source.Cancel();

            // Poll checks for real every few calls, which keeps it cheap in a hot loop; ThrowIfCancelled is
            // the immediate form.
            Should.Throw<OperationCanceledException>(() =>
            {
                for (int i = 0; i < 64; i++) OfficeOperation.Poll();
            });
            Should.Throw<OperationCanceledException>(OfficeOperation.ThrowIfCancelled);
        }
    }

    [Fact]
    public void AScopeWithNoTokenAndNoLimitCannotBeCancelled()
    {
        using (OfficeOperation.Begin())
        {
            OfficeOperation.IsCancellable.ShouldBeFalse();
            OfficeOperation.ThrowIfCancelled();
        }
    }

    [Fact]
    public void ARunThatOutlastsItsLimitThrowsTheTimeoutSubtypeCarryingTheLimit()
    {
        LayoutTimeoutException thrown = Should.Throw<LayoutTimeoutException>(() =>
            OfficeOperation.Run(TimeSpan.FromMilliseconds(20), null, () =>
            {
                while (true)
                {
                    Thread.Sleep(1);
                    OfficeOperation.ThrowIfCancelled();
                }
#pragma warning disable CS0162
                return 0;
#pragma warning restore CS0162
            }));

        thrown.Timeout.ShouldBe(TimeSpan.FromMilliseconds(20));
        thrown.ShouldBeAssignableTo<OperationCanceledException>();
    }

    [Fact]
    public void ACallersCancellationIsNotRelabelledAsATimeout()
    {
        using CancellationTokenSource source = new();
        Exception? thrown = Record.Exception(() =>
            OfficeOperation.Run(TimeSpan.FromMinutes(5), null, () =>
            {
                source.Cancel();
                OfficeOperation.ThrowIfCancelled();
                return 0;
            }, source.Token));

        thrown.ShouldBeOfType<OperationCanceledException>();
    }

    [Fact]
    public void TheLayoutOptionsPictureLimitIsVisibleInsideTheScopeAndOnlyThere()
    {
        OfficeOperation.MaxImagePixels.ShouldBe(0);

        OfficeOperation.Layout(
            new LayoutOptions { MaxImagePixels = 1234, OversizedImages = OversizedImagePolicy.Downscale },
            () =>
            {
                OfficeOperation.MaxImagePixels.ShouldBe(1234);
                OfficeOperation.OversizedImages.ShouldBe(OversizedImagePolicy.Downscale);

                // A nested scope that states none keeps the enclosing layout's.
                using (OfficeOperation.Begin())
                {
                    OfficeOperation.MaxImagePixels.ShouldBe(1234);
                }

                return 0;
            });

        OfficeOperation.MaxImagePixels.ShouldBe(0);
        OfficeOperation.OversizedImages.ShouldBe(OversizedImagePolicy.Skip);
    }

    [Fact]
    public void AScopeIsPerThread()
    {
        using CancellationTokenSource source = new();
        using (OfficeOperation.Begin(cancellationToken: source.Token))
        {
            bool seenElsewhere = true;
            Thread other = new(() => seenElsewhere = OfficeOperation.IsCancellable);
            other.Start();
            other.Join();

            seenElsewhere.ShouldBeFalse();
            OfficeOperation.IsCancellable.ShouldBeTrue();
        }
    }

    [Fact]
    public void APollingStreamPassesBytesThroughAndChecksTheOperationOnEveryRead()
    {
        byte[] data = [.. Enumerable.Range(0, 200).Select(i => (byte)i)];
        using PollingStream stream = new(new MemoryStream(data));

        stream.CanRead.ShouldBeTrue();
        stream.CanSeek.ShouldBeTrue();
        stream.CanWrite.ShouldBeFalse();
        stream.Length.ShouldBe(200);

        byte[] first = new byte[50];
        stream.Read(first, 0, 50).ShouldBe(50);
        first.ShouldBe(data[..50]);

        stream.Seek(190, SeekOrigin.Begin);
        stream.ReadByte().ShouldBe(190);
        stream.Position.ShouldBe(191);

        using CancellationTokenSource source = new();
        using (OfficeOperation.Begin(cancellationToken: source.Token))
        {
            source.Cancel();
            stream.Position = 0;

            // The first reads are not necessarily checked (the counter is what spaces the real checks out),
            // so read until it is: a parser reads far more than 32 times.
            Should.Throw<OperationCanceledException>(() =>
            {
                byte[] buffer = new byte[1];
                for (int i = 0; i < 64; i++)
                {
                    stream.Position = 0;
                    stream.ReadExactly(buffer, 0, 1);
                }
            });
        }

        Should.Throw<NotSupportedException>(() => stream.Write([1], 0, 1));
        Should.Throw<NotSupportedException>(() => stream.SetLength(1));
    }

    [Fact]
    public void ADefaultLayoutHasNoLimits()
    {
        LayoutOptions options = LayoutOptions.Default;

        options.Timeout.ShouldBeNull();
        options.MaxImagePixels.ShouldBe(0);
        options.OversizedImages.ShouldBe(OversizedImagePolicy.Skip);
        options.MaxPages.ShouldBe(0);
    }
}
