using XRay.Office.Containers;
using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Formats;
using XRay.Office.Presentations.OpenDocument;
using XRay.Office.Presentations.Ooxml;

namespace XRay.Office.Presentations;

/// <summary>
/// Reads presentations: PPTX and its variants, legacy PPT, and ODP.
/// The XRay.Office counterpart of LibreOffice Impress.
/// </summary>
/// <remarks>
/// <para>
/// Presentations are the most rendering-dominated of the three families — almost
/// nothing on a slide flows, and almost everything is a positioned shape — so the shape
/// model and the theme resolution around it carry most of the fidelity burden.
/// </para>
/// <para>
/// The property-inheritance chain is where PPTX rendering usually goes wrong: a shape's
/// effective formatting comes from the shape, then its layout placeholder, then the
/// master's placeholder, then the theme's defaults, resolved per text level. Getting
/// that chain wrong makes every themed shape on every slide come out wrong at once. See
/// <c>dotnet/research/04-impress.md</c> section B.
/// </para>
/// </remarks>
public sealed class PresentationReader : IDocumentReader
{
    /// <inheritdoc/>
    public IReadOnlyCollection<DocumentFormat> SupportedFormats { get; } =
    [
        DocumentFormat.Pptx, DocumentFormat.Pptm, DocumentFormat.Potx, DocumentFormat.Potm,
        DocumentFormat.Ppsx, DocumentFormat.Ppsm,
        DocumentFormat.Ppt, DocumentFormat.Pot, DocumentFormat.Pps,
        DocumentFormat.Odp, DocumentFormat.Otp, DocumentFormat.Fodp,
        DocumentFormat.Sxi, DocumentFormat.Sti,
    ];

    /// <inheritdoc/>
    public IDocument Read(DocumentSource source)
    {
        ArgumentNullException.ThrowIfNull(source);
        DocumentFormat format = SourceIdentification.Resolve(source);

        return format switch
        {
            // Wrapped so the deck can be laid out: one OdfDocument serves all three ODF families
            // and only this one paginates into slides.
            DocumentFormat.Odp or DocumentFormat.Otp or DocumentFormat.Fodp
                => new OdpDocument(new OdpReader().Read(source, format)),

            DocumentFormat.Pptx or DocumentFormat.Pptm or DocumentFormat.Potx or DocumentFormat.Potm
                or DocumentFormat.Ppsx or DocumentFormat.Ppsm
                => PptxReader.Read(source, format),

            DocumentFormat.Ppt or DocumentFormat.Pot or DocumentFormat.Pps
                => MsBinary.PptReader.Read(source, format),

            // Named in SupportedFormats but not implemented yet: the OpenOffice.org 1.x presentation
            // forms, which are a different namespace set rather than a different container.
            DocumentFormat.Sxi or DocumentFormat.Sti
                => throw new UnsupportedFormatException(
                    format, $"Reading {format} is not implemented yet."),

            _ => throw new UnsupportedFormatException(format, $"{format} is not a presentation format."),
        };
    }
}
