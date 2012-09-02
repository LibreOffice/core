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

    virtual OString     getWhiteSpace() const   { return m_sWhiteSpace; }
    virtual OString     getExtractCom() const   { return m_sExtractCom; }
    virtual OString     getReference() const    { return m_sReference; }
    virtual OString     getContext() const      { return m_sContext; }
    virtual OString     getUnTransStr() const   { return m_sUnTransStr; }
    virtual OString     getTransStr() const     { return m_sTransStr; }
    virtual bool        getFuzzy() const        { return m_bFuzzy; }

    virtual void        setWhiteSpace(const OString& rWhiteSpace);
    virtual void        setExtractCom(const OString& rExtractCom);
    virtual void        setReference(const OString& rReference);
    virtual void        setContext(const OString& rContext);
    virtual void        setUnTransStr(const OString& rUnTransStr);
    virtual void        setTransStr(const OString& rTransStr);
    virtual void        setFuzzy(const bool bFuzzy);
    virtual void        genKeyId();

    virtual void        writeToFile(std::ofstream& rOFStream);
    virtual void        readFromFile(std::ifstream& rIFStream);
};

class PoEntry
{
public:
    enum SDFPARTS { PROJECT, SOURCEFILE, DUMMY, RESOURCETYPE, GROUPID,
                    LOCALID, HELPID, PLATFORM, WIDTH, LANGUAGEID,
                    TEXT, HELPTEXT, QUICKHELPTEXT, TITLE, TIMESTAMP };
    enum TYPE { TTEXT=TEXT, TQUICKHELPTEXT=QUICKHELPTEXT, TTITLE=TITLE };
private:

    GenPoEntry  m_aGenPo;
    OString     m_sSourceFile;
    OString     m_sGroupId;
    OString     m_sLocalId;
    OString     m_sResourceType;
    TYPE        m_eType;
    OString     m_sHelpText;
public:

                    PoEntry();
                    PoEntry(const OString& i_rSDFLine,
                            const TYPE eType = TTEXT);
    virtual         ~PoEntry();

    OString         getSourceFile() const       { return m_sSourceFile; }
    OString         getGroupId() const          { return m_sGroupId; }
    OString         getLocalId() const          { return m_sLocalId; }
    OString         getResourceType() const     { return m_sResourceType; }
    TYPE            getType() const             { return m_eType; }
    OString         getHelpText() const         { return m_sHelpText; }
    OString         getUnTransStr() const;
    OString         getTransStr() const;
    bool            getFuzzy() const            { return m_aGenPo.getFuzzy(); }
    void            setUnTransStr(const OString& rUnTransStr);
    void            setTransStr(const OString& rTransStr);
    void            setFuzzy(const bool bFuzzy);

    void            writeToFile(std::ofstream& rOFStream);
    void            readFromFile(std::ifstream& rIFStream);

};

class PoHeader
{

private:
    GenPoEntry m_aGenPo;
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
    void            writeToFile(std::ofstream& rOFStream);
};

#endif // _PO_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
