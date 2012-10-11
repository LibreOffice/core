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
#include <boost/noncopyable.hpp>

class PoOfstream;
class PoIfstream;
class GenPoEntry;

class PoEntry
{
private:

    GenPoEntry* m_pGenPo;
    bool m_bIsInitialized;

public:

    friend class PoOfstream;
    friend class PoIfstream;

    enum SDFPART { PROJECT, SOURCEFILE, DUMMY, RESOURCETYPE, GROUPID,
                    LOCALID, HELPID, PLATFORM, WIDTH, LANGUAGEID,
                    TEXT, HELPTEXT, QUICKHELPTEXT, TITLE, TIMESTAMP };
    enum TYPE { TTEXT=TEXT, TQUICKHELPTEXT=QUICKHELPTEXT, TTITLE=TITLE };
    enum Exception { INVALIDSDFLINE };

                    PoEntry();
                    PoEntry(const OString& rSDFLine,
                            const TYPE eType = TTEXT);
                    ~PoEntry();

                    PoEntry( const PoEntry& rPo );
    PoEntry&        operator=( const PoEntry& rPo );

    OString         getSourceFile() const;
    OString         getGroupId() const;
    OString         getLocalId() const;
    OString         getResourceType() const;
    TYPE            getType() const;
    OString         getMsgId() const;
    OString         getMsgStr() const;
    bool            getFuzzy() const;
    OString         getKeyId() const;
    void            setMsgId(const OString& rUnTransStr);
    void            setMsgStr(const OString& rTransStr);
    void            setFuzzy(const bool bFuzzy);

    static bool     IsInSameComp(const PoEntry& rPo1,const PoEntry& rPo2);

};

class PoHeader: private boost::noncopyable
{
private:

    GenPoEntry* m_pGenPo;
    bool m_bIsInitialized;

public:

    friend class PoOfstream;
    friend class PoIfstream;

    enum Exception { NOLANG };

                    PoHeader();
                    PoHeader( const OString& rExtSrc );
                    PoHeader( std::ifstream& rOldPo );
                    ~PoHeader();

    OString         getLanguage() const;
};

class PoOfstream: private boost::noncopyable
{
private:

    std::ofstream   m_aOutPut;
    bool            m_bIsAfterHeader;

public:
            PoOfstream();
            ~PoOfstream();
    bool    isOpen() const  { return m_aOutPut.is_open(); }

    void    open(const OString& rFileName);
    void    close();
    void    writeHeader(const PoHeader& rHeader);
    void    writeEntry(const PoEntry& rPo);
};

class PoIfstream: private boost::noncopyable
{
private:

    std::ifstream   m_aInPut;
    bool            m_bIsAfterHeader;
    bool            m_bEof;

public:

    enum Exception { INVALIDENTRY, INVALIDHEADER };

            PoIfstream();
            ~PoIfstream();
    bool    isOpen() const  { return m_aInPut.is_open(); }
    bool    eof() const     { return m_bEof; }

    void    open(const OString& rFileName);
    void    close();
    void    readHeader(PoHeader& rHeader);
    void    readEntry(PoEntry& rPo);
};

#endif // _PO_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
