using Paperless.Containers;
using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Formats;
using Paperless.Presentations.OpenDocument;
using Paperless.Presentations.Ooxml;

namespace Paperless.Presentations;

/// <summary>
/// Reads presentations: PPTX and its variants, legacy PPT, and ODP.
/// The Paperless counterpart of LibreOffice Impress.
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
            DocumentFormat.Odp or DocumentFormat.Otp or DocumentFormat.Fodp
                => new OdpReader().Read(source, format),

            DocumentFormat.Pptx or DocumentFormat.Pptm or DocumentFormat.Potx or DocumentFormat.Potm
                or DocumentFormat.Ppsx or DocumentFormat.Ppsm
                => PptxReader.Read(source, format),

            // Named in SupportedFormats but not implemented yet.
            DocumentFormat.Ppt or DocumentFormat.Pot or DocumentFormat.Pps
                or DocumentFormat.Sxi or DocumentFormat.Sti
                => throw new UnsupportedFormatException(
                    format, $"Reading {format} is not implemented yet."),

            _ => throw new UnsupportedFormatException(format, $"{format} is not a presentation format."),
        };
    }
}
