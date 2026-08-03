using System.Xml.Linq;
using Paperless.WordProcessing.Ooxml;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Which runs a word-processing document asks to have their pairs kerned.
/// </summary>
/// <remarks>
/// <para>
/// The default is <em>off</em>, and that is the whole point of reading the property. A Writer document
/// starts from the pool default of <c>SvxAutoKernItem(false)</c>
/// (<c>sw/source/core/bastyp/init.cxx:300</c>) and <c>SwDocShell::Load</c> resets the user default back
/// to it on every load through <c>RemoveAllFormatLanguageDependencies</c>
/// (<c>sw/source/uibase/app/docsh.cxx:228</c>, <c>sw/source/core/doc/poolfmt.cxx:321</c>). The DOCX
/// importer states it again for its own defaults, citing that same function
/// (<c>sw/source/writerfilter/dmapper/StyleSheetTable.cxx:354</c>).
/// </para>
/// <para>
/// <c>w:kern</c> is not a switch: its value is a font size in half-points, at or above which Word kerns.
/// LibreOffice has nowhere to put the threshold — <c>RES_CHRATR_AUTOKERN</c> is a plain boolean — so it
/// keeps only whether one was stated, and says so in a comment beside the line that does it:
/// <c>// auto kerning is bound to a minimum font size in Word - but not in Writer :-(</c>, then
/// <c>rContext-&gt;Insert(PROP_CHAR_AUTO_KERNING, uno::Any(nIntValue != 0))</c>
/// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx:2482</c>). The reference renderings this project
/// is measured against are that binary's, so the threshold is discarded here too.
/// </para>
/// </remarks>
public sealed class AutoKerningTests
{
    [Fact]
    public void ARunSayingNothingIsNotKerned()
        => Resolve(runProperties: null).AutoKerning.ShouldBeFalse();

    [Fact]
    public void AZeroThresholdSwitchesKerningOff()
        => Resolve(Run(kern: "0")).AutoKerning.ShouldBeFalse();

    [Fact]
    public void ANonZeroThresholdSwitchesKerningOn()
        => Resolve(Run(kern: "2")).AutoKerning.ShouldBeTrue();

    [Fact]
    public void AThresholdAboveTheRunsOwnSizeStillSwitchesKerningOn()
    {
        // 14 pt stated as the floor on an 8 pt run. Word would leave this run alone; LibreOffice kerns
        // it, having thrown the threshold away, and LibreOffice is what the corpus is measured against.
        Resolve(Run(kern: "28", size: "16")).AutoKerning.ShouldBeTrue();
    }

    [Fact]
    public void AStyleCanSwitchKerningOnForTheRunsThatNameIt()
    {
        WordStyles styles = Styles(
            new XElement(
                W + "style",
                new XAttribute(W + "type", "paragraph"),
                new XAttribute(W + "styleId", "Kerned"),
                new XElement(W + "rPr", new XElement(W + "kern", new XAttribute(W + "val", "32")))));

        WordTextStyle text = WordParagraphFormats.ResolveRun(
            styles,
            new XElement(
                W + "pPr",
                new XElement(W + "pStyle", new XAttribute(W + "val", "Kerned"))),
            runProperties: null);

        text.AutoKerning.ShouldBeTrue();
    }

    [Fact]
    public void ARunsOwnZeroBeatsTheStyleThatSwitchedItOn()
    {
        WordStyles styles = Styles(
            new XElement(
                W + "style",
                new XAttribute(W + "type", "paragraph"),
                new XAttribute(W + "styleId", "Kerned"),
                new XElement(W + "rPr", new XElement(W + "kern", new XAttribute(W + "val", "32")))));

        WordTextStyle text = WordParagraphFormats.ResolveRun(
            styles,
            new XElement(
                W + "pPr",
                new XElement(W + "pStyle", new XAttribute(W + "val", "Kerned"))),
            Run(kern: "0"));

        text.AutoKerning.ShouldBeFalse();
    }

    [Fact]
    public void TheDocumentDefaultsReachARunThatNamesNoStyle()
    {
        WordStyles styles = Styles();
        styles.Add(
            new XElement(
                W + "styles",
                new XElement(
                    W + "docDefaults",
                    new XElement(
                        W + "rPrDefault",
                        new XElement(
                            W + "rPr",
                            new XElement(W + "kern", new XAttribute(W + "val", "2")))))));

        WordParagraphFormats.ResolveRun(styles, null, null).AutoKerning.ShouldBeTrue();
    }

    private static readonly XNamespace W =
        "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    private static XElement Run(string kern, string? size = null)
    {
        XElement properties = new(W + "rPr", new XElement(W + "kern", new XAttribute(W + "val", kern)));
        if (size is not null)
        {
            properties.Add(new XElement(W + "sz", new XAttribute(W + "val", size)));
        }

        return properties;
    }

    private static WordStyles Styles(params XElement[] styles)
    {
        WordStyles read = new();
        read.Add(new XElement(W + "styles", styles));
        return read;
    }

    private static WordTextStyle Resolve(XElement? runProperties)
        => WordParagraphFormats.ResolveRun(Styles(), null, runProperties);
}
