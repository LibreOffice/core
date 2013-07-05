/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _RTFLISTENER_HXX_
#define _RTFLISTENER_HXX_

#include <rtfcontrolwords.hxx>

namespace writerfilter {
    namespace rtftok {
        enum RTFInternalState
        {
            INTERNAL_NORMAL,
            INTERNAL_BIN,
            INTERNAL_HEX
        };

        // Note that this is not a 1:1 mapping between destination control
        // words, e.g. RTF_PICT gets mapped to DESTINATION_PICT or
        // DESTINATION_SHAPEPROPERTYVALUEPICT, etc.
        enum RTFDestinationState
        {
            DESTINATION_NORMAL,
            DESTINATION_SKIP,
            DESTINATION_FONTTABLE,
            DESTINATION_FONTENTRY,
            DESTINATION_COLORTABLE,
            DESTINATION_STYLESHEET,
            DESTINATION_STYLEENTRY,
            DESTINATION_FIELD,
            DESTINATION_FIELDINSTRUCTION,
            DESTINATION_FIELDRESULT,
            DESTINATION_LISTTABLE,
            DESTINATION_LISTPICTURE,
            DESTINATION_LISTENTRY,
            DESTINATION_LISTOVERRIDETABLE,
            DESTINATION_LISTOVERRIDEENTRY,
            DESTINATION_LISTLEVEL,
            DESTINATION_LEVELTEXT,
            DESTINATION_LEVELNUMBERS,
            DESTINATION_SHPPICT,
            DESTINATION_PICT,
            DESTINATION_PICPROP,
            DESTINATION_SHAPEPROPERTY,
            DESTINATION_SHAPEPROPERTYNAME,
            DESTINATION_SHAPEPROPERTYVALUE,
            DESTINATION_SHAPE,
            DESTINATION_SHAPEINSTRUCTION,
            DESTINATION_SHAPEPROPERTYVALUEPICT,
            DESTINATION_NESTEDTABLEPROPERTIES,
            DESTINATION_FOOTNOTE,
            DESTINATION_BOOKMARKSTART,
            DESTINATION_BOOKMARKEND,
            DESTINATION_REVISIONTABLE,
            DESTINATION_REVISIONENTRY,
            DESTINATION_SHAPETEXT,
            DESTINATION_FORMFIELD,
            DESTINATION_FORMFIELDNAME,
            DESTINATION_FORMFIELDLIST,
            DESTINATION_DATAFIELD,
            DESTINATION_INFO,
            DESTINATION_CREATIONTIME,
            DESTINATION_REVISIONTIME,
            DESTINATION_PRINTTIME,
            DESTINATION_AUTHOR,
            DESTINATION_KEYWORDS,
            DESTINATION_OPERATOR,
            DESTINATION_COMPANY,
            DESTINATION_COMMENT,
            DESTINATION_OBJECT,
            DESTINATION_OBJDATA,
            DESTINATION_RESULT,
            DESTINATION_ANNOTATIONDATE,
            DESTINATION_ANNOTATIONAUTHOR,
            DESTINATION_FALT,
            DESTINATION_FLYMAINCONTENT,
            DESTINATION_DRAWINGOBJECT,
            DESTINATION_PARAGRAPHNUMBERING,
            DESTINATION_PARAGRAPHNUMBERING_TEXTBEFORE,
            DESTINATION_PARAGRAPHNUMBERING_TEXTAFTER,
            DESTINATION_TITLE,
            DESTINATION_SUBJECT,
            DESTINATION_DOCCOMM,
            DESTINATION_ATNID,
            DESTINATION_MOMATH,
            DESTINATION_MR,
            DESTINATION_MF,
            DESTINATION_MFPR,
            DESTINATION_MCTRLPR,
            DESTINATION_MNUM,
            DESTINATION_MDEN,
            DESTINATION_MACC,
            DESTINATION_MACCPR,
            DESTINATION_MCHR,
            DESTINATION_MPOS,
            DESTINATION_MVERTJC,
            DESTINATION_MSTRIKEH,
            DESTINATION_MDEGHIDE,
            DESTINATION_ME,
            DESTINATION_MBAR,
            DESTINATION_MBARPR,
            DESTINATION_MD,
            DESTINATION_MDPR,
            DESTINATION_MBEGCHR,
            DESTINATION_MSEPCHR,
            DESTINATION_MENDCHR,
            DESTINATION_MFUNC,
            DESTINATION_MFUNCPR,
            DESTINATION_MFNAME,
            DESTINATION_MLIMLOW,
            DESTINATION_MLIMLOWPR,
            DESTINATION_MLIM,
            DESTINATION_MM,
            DESTINATION_MMPR,
            DESTINATION_MMR,
            DESTINATION_MNARY,
            DESTINATION_MNARYPR,
            DESTINATION_MSUB,
            DESTINATION_MSUP,
            DESTINATION_MSUBHIDE,
            DESTINATION_MSUPHIDE,
            DESTINATION_MLIMUPP,
            DESTINATION_MLIMUPPPR,
            DESTINATION_MGROUPCHR,
            DESTINATION_MGROUPCHRPR,
            DESTINATION_MBORDERBOX,
            DESTINATION_MBORDERBOXPR,
            DESTINATION_MRAD,
            DESTINATION_MRADPR,
            DESTINATION_MDEG,
            DESTINATION_MSSUB,
            DESTINATION_MSSUBPR,
            DESTINATION_MSSUP,
            DESTINATION_MSSUPPR,
            DESTINATION_MSSUBSUP,
            DESTINATION_MSSUBSUPPR,
            DESTINATION_MSPRE,
            DESTINATION_MSPREPR,
            DESTINATION_MTYPE,
            DESTINATION_MGROW,
            DESTINATION_MBOX,
            DESTINATION_MEQARR,
            DESTINATION_UPR,
            DESTINATION_LFOLEVEL,
            DESTINATION_BACKGROUND,
            DESTINATION_SHAPEGROUP,
        };

        enum RTFErrors
        {
            ERROR_OK,
            ERROR_GROUP_UNDER,
            ERROR_GROUP_OVER,
            ERROR_EOF,
            ERROR_HEX_INVALID,
            ERROR_CHAR_OVER
        };

        /**
         * RTFTokenizer needs a class implementing this this interface. While
         * RTFTokenizer separates control words (and their arguments) from
         * text, the class implementing this interface is expected to map the
         * raw RTF tokens to dmapper tokens.
         */
        class RTFListener
        {
            public:
                virtual ~RTFListener() { }
                // Dispatching of control words and characters.
                virtual int dispatchDestination(RTFKeyword nKeyword) = 0;
                virtual int dispatchFlag(RTFKeyword nKeyword) = 0;
                virtual int dispatchSymbol(RTFKeyword nKeyword) = 0;
                virtual int dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam) = 0;
                virtual int dispatchValue(RTFKeyword nKeyword, int nParam) = 0;
                virtual int resolveChars(char ch) = 0;

                // State handling.
                virtual int pushState() = 0;
                virtual int popState() = 0;

                virtual RTFDestinationState getDestinationState() = 0;
                virtual void setDestinationState(RTFDestinationState nDestinationState) = 0;
                virtual RTFInternalState getInternalState() = 0;
                virtual void setInternalState(RTFInternalState nInternalState) = 0;
                virtual bool getSkipUnknown() = 0;
                virtual void setSkipUnknown(bool bSkipUnknown) = 0;

                // Substream handling.
                virtual void finishSubstream() = 0;
                virtual bool isSubstream() const = 0;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFLISTENER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
