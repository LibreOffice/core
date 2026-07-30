namespace Paperless.MsBinary.Escher;

/// <summary>
/// Escher (MS-ODRAW) is the shared drawing-object format embedded inside DOC, XLS and
/// PPT. Every shape, picture, text box and grouping in a legacy binary file is an
/// Escher record.
/// </summary>
/// <remarks>
/// Because all three legacy formats delegate their drawings to it, implementing Escher
/// once buys shape support across all of them — the same leverage LibreOffice gets from
/// <c>filter/source/msfilter/msdffimp.cxx</c>. Shape properties arrive as a sparse
/// property table (the <c>DFF_Prop_*</c> identifiers) rather than as fixed fields, so
/// the decoder is driven by a property-id lookup. See
/// <c>dotnet/research/04-impress.md</c> section C.
/// </remarks>
public static class EscherRecordTypes
{
    /// <summary>Container for a drawing group; the root of a file's drawing data.</summary>
    public const ushort DrawingGroupContainer = 0xF000;

    /// <summary>The blip (picture) store container.</summary>
    public const ushort BlipStoreContainer = 0xF001;

    /// <summary>Container for one page or sheet's drawing.</summary>
    public const ushort DrawingContainer = 0xF002;

    /// <summary>Container for a shape group.</summary>
    public const ushort ShapeGroupContainer = 0xF003;

    /// <summary>Container for a single shape.</summary>
    public const ushort ShapeContainer = 0xF004;

    /// <summary>The shape's type and identifier.</summary>
    public const ushort Shape = 0xF00A;

    /// <summary>The sparse property table carrying most of a shape's formatting.</summary>
    public const ushort ShapeProperties = 0xF00B;

    /// <summary>The shape's bounding box, for child shapes of a group.</summary>
    public const ushort ChildAnchor = 0xF00F;

    /// <summary>The shape's bounding box, for top-level shapes.</summary>
    public const ushort ClientAnchor = 0xF010;

    /// <summary>Host-application data attached to a shape.</summary>
    public const ushort ClientData = 0xF011;
}
