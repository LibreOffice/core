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
    bool       m_bNull;
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
    virtual bool        isNull() const          { return m_bNull; }
    virtual OString     getKeyId() const        { return m_sKeyId; }

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
private:
    GenPoEntry m_aGenPo;

public:
    enum SDFPARTS { PROJECT, SOURCEFILE, DUMMY, RESOURCETYPE, GROUPID,
                    LOCALID, HELPID, PLATFORM, WIDTH, LANGUAGEID,
                    TEXT, HELPTEXT, QUICKHELPTEXT, TITLE, TIMESTAMP };
    enum TYPE { TTEXT=TEXT, TQUICKHELPTEXT=QUICKHELPTEXT, TTITLE=TITLE };

                    PoEntry();
                    PoEntry(const OString& rSDFLine,
                            const TYPE eType = TTEXT);
                    ~PoEntry();

    OString         getSourceFile() const;
    OString         getGroupId() const;
    OString         getLocalId() const;
    OString         getResourceType() const;
    TYPE            getType() const;
    OString         getUnTransStr() const;
    OString         getTransStr() const;
    bool            getFuzzy() const;
    bool            isNull() const;
    OString         getKeyId() const;
    void            setUnTransStr(const OString& rUnTransStr);
    void            setTransStr(const OString& rTransStr);
    void            setFuzzy(const bool bFuzzy);

    void            writeToFile(std::ofstream& rOFStream);
    void            readFromFile(std::ifstream& rIFStream);

    static bool     IsInSameComp(const PoEntry& rPo1,const PoEntry& rPo2);

};

class PoHeader
{
private:
    GenPoEntry m_aGenPo;

public:
                    PoHeader();
                    PoHeader( const OString& rExtSrc );
                    ~PoHeader();

    OString         getLanguage() const;
    void            writeToFile(std::ofstream& rOFStream);
    void            readFromFile(std::ifstream& rIFStream);
};

#endif // _PO_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
