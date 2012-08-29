/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
