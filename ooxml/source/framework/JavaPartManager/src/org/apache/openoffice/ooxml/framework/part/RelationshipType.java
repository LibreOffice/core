package org.apache.openoffice.ooxml.framework.part;

public enum RelationshipType
{
    ExtendedProperties,
    CoreProperties,
    OfficeDocument,
    Image,
    Header,
    Hyperlink,
    Styles,
    EndNotes,
    Footer,
    Numbering,
    CustomXML,
    FootNotes,
    WebSettings,
    Theme,
    Settings,
    FontTable,
    Thumbnail,
    Slide,
    ViewProperties,
    PresentationProperties,
    HandoutMaster,
    TableStyles,
    SlideMaster,
    NotesMaster,
    SlideLayout,
    NotesSlide,
    VMLDrawing,
    OLE,
    Chart,
    Package,
    ThemeOverride,

    Unknown
    ;

    public static RelationshipType CreateFromString (final String sType)
    {
        switch(sType)
        {
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties":
                return ExtendedProperties;
            case "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties":
                return CoreProperties;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument":
                return OfficeDocument;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image":
                return Image;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/header":
                return Header;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink":
                return Hyperlink;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles":
                return Styles;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/endnotes":
                return EndNotes;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer":
                return Footer;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering":
                return Numbering;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXml":
                return CustomXML;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes":
                return FootNotes;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/webSettings":
                return WebSettings;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme":
                return Theme;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings":
                return Settings;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable":
                return FontTable;
            case "http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail":
                return Thumbnail;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide":
                return Slide;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/viewProps":
                return ViewProperties;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/presProps":
                return PresentationProperties;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/handoutMaster":
                return HandoutMaster;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/tableStyles":
                return TableStyles;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster":
                return SlideMaster;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/notesMaster":
                return NotesMaster;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout":
                return SlideLayout;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/notesSlide":
                return NotesSlide;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/vmlDrawing":
                return VMLDrawing;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject":
                return OLE;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart":
                return Chart;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package":
                return Package;
            case "http://schemas.openxmlformats.org/officeDocument/2006/relationships/themeOverride":
                return ThemeOverride;

            default:
                System.err.printf(sType +" is not yet supported\n");
                return Unknown;
        }
    }
}
