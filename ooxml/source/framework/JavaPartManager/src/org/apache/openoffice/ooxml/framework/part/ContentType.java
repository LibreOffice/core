package org.apache.openoffice.ooxml.framework.part;

public enum ContentType
{
    ApplicationDrawing  ("application/vnd.openxmlformats-officedocument.vmlDrawing"),
    ApplicationExcel    ("application/vnd.ms-excel"),
    ApplicationXML      ("application/xml"),
    Chart               ("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"),
    ContentTypes        (""),
    CoreProperties      ("application/vnd.openxmlformats-package.core-properties+xml"),
    CustomXMLProperties ("application/vnd.openxmlformats-officedocument.customXmlProperties+xml"),
    ExtendedProperties  ("application/vnd.openxmlformats-officedocument.extended-properties+xml"),
    ImageGIF            ("image/gif"),
    ImageJPG            ("image/png"),
    ImagePNG            ("image/jpeg"),
    OleObject           ("application/vnd.openxmlformats-officedocument.oleObject"),
    PmlDocument         ("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"),
    PmlHandoutMaster    ("application/vnd.openxmlformats-officedocument.presentationml.handoutMaster+xml"),
    PmlNotesMaster      ("application/vnd.openxmlformats-officedocument.presentationml.notesMaster+xml"),
    PmlNotesSlide       ("application/vnd.openxmlformats-officedocument.presentationml.notesSlide+xml"),
    PmlProperties       ("application/vnd.openxmlformats-officedocument.presentationml.presProps+xml"),
    PmlSlide            ("application/vnd.openxmlformats-officedocument.presentationml.slide+xml"),
    PmlSlideLayout      ("application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"),
    PmlSlideMaster      ("application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"),
    PmlTableStyles      ("application/vnd.openxmlformats-officedocument.presentationml.tableStyles+xml"),
    PmlViewProperties   ("application/vnd.openxmlformats-officedocument.presentationml.viewProps+xml"),
    Relationships       ("application/vnd.openxmlformats-package.relationships+xml"),
    SmlSheet            ("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"),
    Theme               ("application/vnd.openxmlformats-officedocument.theme+xml"),
    ThemeOverride       ("application/vnd.openxmlformats-officedocument.themeOverride+xml"),
    Thumbnail           ("http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail"),
    WmlDocument         ("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"),
    WmlEndNotes         ("application/vnd.openxmlformats-officedocument.wordprocessingml.endnotes+xml"),
    WmlFontTable        ("application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml"),
    WmlFootNotes        ("application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml"),
    WmlFooter           ("application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"),
    WmlHeader           ("application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"),
    WmlNumbering        ("application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"),
    WmlSettings         ("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"),
    WmlStyles           ("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"),
    WmlWebSettings      ("application/vnd.openxmlformats-officedocument.wordprocessingml.webSettings+xml"),

    Unknown("");




    ContentType (final String sMimeType)
    {
        msMimeType = sMimeType;
    }




    public static ContentType CreateForString (final String sContentType)
    {
        for (final ContentType eType : values())
            if (eType.msMimeType.equals(sContentType))
                return eType;
        System.err.printf("content type '%s' is not known\n", sContentType);
        return Unknown;
    }




    public String GetLongName ()
    {
        return msMimeType;
    }




    private final String msMimeType;
}
