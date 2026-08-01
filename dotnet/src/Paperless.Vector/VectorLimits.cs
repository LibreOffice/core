namespace Paperless.Vector;

/// <summary>
/// Caps on the work a single embedded vector image may cause.
/// </summary>
/// <remarks>
/// <para>
/// These streams arrive inside documents that arrive by e-mail, so they are attacker
/// controlled and none of the formats bounds itself. WMF, EMF and EMF+ are command streams
/// that can be made to loop or to allocate; SVG's <c>&lt;use&gt;</c> is a macro expansion
/// with no fan-out limit at all. The same reasoning already produced the ZIP guards in
/// <c>Paperless.Containers.Zip.ZipPackageBase.Limits</c> and the
/// <c>DtdProcessing.Prohibit</c> settings in the XML readers; this is that idea applied to
/// the third untrusted input.
/// </para>
/// <para>
/// <b>The fan-out cap is the one that earns its keep.</b> A 1 057-byte SVG of thirteen
/// nested <c>&lt;g&gt;</c> elements, each holding three <c>&lt;use&gt;</c> references to the
/// one below, expands to 3<sup>13</sup> ≈ 1.6 million drawn nodes; measured against
/// <c>Svg.SceneGraph</c> 5.1.1 it had not finished after <b>120 seconds</b> and was still
/// allocating. The byte cap cannot see it and a command cap cannot either, because the
/// explosion happens inside the library's scene compiler before a single command is
/// produced. Only counting the expansion <em>before</em> handing the document over catches
/// it — which is why <see cref="MaxExpandedNodes"/> exists and why
/// <see cref="Vector.Svg.SvgExpansionGuard"/> runs first.
/// </para>
/// </remarks>
public sealed record VectorLimits
{
    /// <summary>The defaults, generous enough for any real embedded graphic.</summary>
    public static readonly VectorLimits Default = new();

    /// <summary>
    /// Largest encoded image to attempt, in bytes. Real embedded vector art is tens of
    /// kilobytes; 32 MiB is far past anything legitimate.
    /// </summary>
    public int MaxBytes { get; init; } = 32 * 1024 * 1024;

    /// <summary>
    /// Largest number of drawing commands to record. Beyond this the image is truncated and
    /// a diagnostic is raised, rather than the caller waiting indefinitely.
    /// </summary>
    public int MaxCommands { get; init; } = 250_000;

    /// <summary>
    /// Largest number of path segments to record across the whole image. A command cap alone
    /// does not bound the work: one path may carry millions of points.
    /// </summary>
    public int MaxPathSegments { get; init; } = 4_000_000;

    /// <summary>
    /// Largest nesting depth of saved states and nested pictures. Guards the recursive walk
    /// of a nested command list against a stack overflow, which is not catchable.
    /// </summary>
    public int MaxNestingDepth { get; init; } = 256;

    /// <summary>
    /// Largest number of source nodes a document may expand to once macro-style references
    /// (SVG's <c>&lt;use&gt;</c>) are instantiated. See the remarks on this type.
    /// </summary>
    public int MaxExpandedNodes { get; init; } = 200_000;

    /// <summary>
    /// Wall-clock budget for decoding one image.
    /// </summary>
    /// <remarks>
    /// Checked at every command the translator records, so it bounds the half of the work
    /// Paperless drives. It cannot interrupt the third-party parser, which is precisely why
    /// <see cref="MaxExpandedNodes"/> has to bound that half up front instead.
    /// </remarks>
    public TimeSpan TimeLimit { get; init; } = TimeSpan.FromSeconds(10);
}
