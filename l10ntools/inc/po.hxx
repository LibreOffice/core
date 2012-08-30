/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _PO_INCLUDED
#define _PO_INCLUDED

#include <fstream>
#include <rtl/string.hxx>

class GenPoEntry
{

private:

    OString    m_sWhiteSpace;
    OString    m_sExtractCom;
    OString    m_sReference;
    OString    m_sContext;
    OString    m_sUnTransStr;
    OString    m_sTransStr;
    bool       m_bFuzzy;
    OString    m_sKeyId;

public:
                        GenPoEntry();
    virtual             ~GenPoEntry();

    virtual void        setWhiteSpace(const OString& rWhiteSpace)
                        { m_sWhiteSpace = rWhiteSpace; }
    virtual void        setExtractCom(const OString& rExtractCom)
                        { m_sExtractCom = rExtractCom; }
    virtual void        setReference(const OString& rReference)
                        { m_sReference = rReference; }
    virtual void        setContext(const OString& rContext)
                        { m_sContext = rContext; }
    virtual void        setUnTransStr(const OString& rUnTransStr)
                        { m_sUnTransStr = rUnTransStr; }
    virtual void        setTransStr(const OString& rTransStr)
                        { m_sTransStr = rTransStr; }
    virtual void        setFuzzy(bool bFuzzy)
                        { m_bFuzzy = bFuzzy; }
    virtual void        genKeyId();
    virtual void        writeToFile(std::ofstream& io_rOFStream);
};

class PoEntry: public GenPoEntry
{
public:
    enum SDFPARTS { PROJECT, SOURCEFILE, DUMMY, RESOURCETYPE, GROUPID,
                    LOCALID, HELPID, PLATFORM, WIDTH, LANGUAGEID,
                    TEXT, HELPTEXT, QUICKHELPTEXT, TITLE, TIMESTAMP };
    enum TYPE { TTEXT=TEXT, TQUICKHELPTEXT=QUICKHELPTEXT, TTITLE=TITLE };
private:

    OString     m_sSourceFile;
    OString     m_sGroupId;
    OString     m_sLocalId;
    OString     m_sResourceType;
    TYPE        m_eType;
    OString     m_sHelpText;

public:

                        PoEntry(const OString& i_rSDFLine,
                            const TYPE eType = TTEXT);
    virtual             ~PoEntry();

    virtual void        setUnTransStr(const OString& rUnTransStr);
    virtual void        setTransStr(const OString& rTransStr);

};

class PoHeader: public GenPoEntry
{

private:
    OString    m_sExtractionSource;
    OString    m_sProjectIdVersion;
    OString    m_sReportMsgidBugsTo;
    OString    m_sPotCreationDate;
    OString    m_sPoRevisionDate;
    OString    m_sLastTranslator;
    OString    m_sLanguageTeam;
    OString    m_sMimeVersion;
    OString    m_sPluralForms;
    OString    m_sContentType;
    OString    m_sCharset;
    OString    m_sEncoding;
    OString    m_sXGenerator;
    OString    m_sXAcceleratorMarker;

public:
                    PoHeader( const OString& rExtSrc );
                    ~PoHeader();
};

#endif // _PO_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
