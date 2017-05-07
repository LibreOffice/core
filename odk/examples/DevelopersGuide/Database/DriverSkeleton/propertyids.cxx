/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include "propertyids.hxx"
namespace connectivity
{
namespace skeleton
{
        const sal_Char* getPROPERTY_QUERYTIMEOUT()          { return    "QueryTimeOut"; }
        const sal_Char* getPROPERTY_MAXFIELDSIZE()          { return    "MaxFieldSize"; }
        const sal_Char* getPROPERTY_MAXROWS()               { return    "MaxRows"; }
        const sal_Char* getPROPERTY_CURSORNAME()            { return    "CursorName"; }
        const sal_Char* getPROPERTY_RESULTSETCONCURRENCY()  { return    "ResultSetConcurrency"; }
        const sal_Char* getPROPERTY_RESULTSETTYPE()         { return    "ResultSetType"; }
        const sal_Char* getPROPERTY_FETCHDIRECTION()        { return    "FetchDirection"; }
        const sal_Char* getPROPERTY_FETCHSIZE()             { return    "FetchSize"; }
        const sal_Char* getPROPERTY_ESCAPEPROCESSING()      { return    "EscapeProcessing"; }
        const sal_Char* getPROPERTY_USEBOOKMARKS()          { return    "UseBookmarks"; }

        const sal_Char* getPROPERTY_NAME()                  { return    "Name"; }
        const sal_Char* getPROPERTY_TYPE()                  { return    "Type"; }
        const sal_Char* getPROPERTY_TYPENAME()              { return    "TypeName"; }
        const sal_Char* getPROPERTY_PRECISION()             { return    "Precision"; }
        const sal_Char* getPROPERTY_SCALE()                 { return    "Scale"; }
        const sal_Char* getPROPERTY_ISNULLABLE()            { return    "IsNullable"; }
        const sal_Char* getPROPERTY_ISAUTOINCREMENT()       { return    "IsAutoIncrement"; }
        const sal_Char* getPROPERTY_ISROWVERSION()          { return    "IsRowVersion"; }
        const sal_Char* getPROPERTY_DESCRIPTION()           { return    "Description"; }
        const sal_Char* getPROPERTY_DEFAULTVALUE()          { return    "DefaultValue"; }

        const sal_Char* getPROPERTY_REFERENCEDTABLE()       { return    "ReferencedTable"; }
        const sal_Char* getPROPERTY_UPDATERULE()            { return    "UpdateRule"; }
        const sal_Char* getPROPERTY_DELETERULE()            { return    "DeleteRule"; }
        const sal_Char* getPROPERTY_CATALOG()               { return    "Catalog"; }
        const sal_Char* getPROPERTY_ISUNIQUE()              { return    "IsUnique"; }
        const sal_Char* getPROPERTY_ISPRIMARYKEYINDEX()     { return    "IsPrimaryKeyIndex"; }
        const sal_Char* getPROPERTY_ISCLUSTERED()           { return    "IsClustered"; }
        const sal_Char* getPROPERTY_ISASCENDING()           { return    "IsAscending"; }
        const sal_Char* getPROPERTY_SCHEMANAME()            { return    "SchemaName"; }
        const sal_Char* getPROPERTY_CATALOGNAME()           { return    "CatalogName"; }
        const sal_Char* getPROPERTY_COMMAND()               { return    "Command"; }
        const sal_Char* getPROPERTY_CHECKOPTION()           { return    "CheckOption"; }
        const sal_Char* getPROPERTY_PASSWORD()              { return    "Password"; }
        const sal_Char* getPROPERTY_RELATEDCOLUMN()         { return    "RelatedColumn"; }

        const sal_Char* getSTAT_INVALID_INDEX()             { return    "Invalid descriptor index"; }

        const sal_Char* getPROPERTY_FUNCTION()              { return    "Function"; }
        const sal_Char* getPROPERTY_TABLENAME()             { return    "TableName"; }
        const sal_Char* getPROPERTY_REALNAME()              { return    "RealName"; }
        const sal_Char* getPROPERTY_DBASEPRECISIONCHANGED() { return    "DbasePrecisionChanged"; }
        const sal_Char* getPROPERTY_ISCURRENCY()            { return    "IsCurrency"; }
        const sal_Char* getPROPERTY_ISBOOKMARKABLE()        { return    "IsBookmarkable"; }

        const sal_Char* getPROPERTY_FORMATKEY()             { return    "FormatKey"; }
        const sal_Char* getPROPERTY_LOCALE()                { return    "Locale"; }

        const sal_Char* getPROPERTY_AUTOINCREMENTCREATION() { return    "AutoIncrementCreation"; }
        const sal_Char* getPROPERTY_PRIVILEGES()            { return    "Privileges"; }

    //= error messages

        const sal_Char* getERRORMSG_SEQUENCE()              { return    "Function sequence error"; }
        const sal_Char* getSQLSTATE_SEQUENCE()              { return    "HY010"; }
        const sal_Char* getSQLSTATE_GENERAL()               { return    "HY0000"; }
        const sal_Char* getSTR_DELIMITER()                  { return    "/"; }

        OPropertyMap::~OPropertyMap()
        {
            ::std::map<sal_Int32 , rtl_uString*>::iterator aIter = m_aPropertyMap.begin();
            for(;aIter != m_aPropertyMap.end();++aIter)
                if(aIter->second)
                    rtl_uString_release(aIter->second);
        }

        ::rtl::OUString OPropertyMap::getNameByIndex(sal_Int32 _nIndex) const
        {
            ::rtl::OUString sRet;
            ::std::map<sal_Int32 , rtl_uString*>::const_iterator aIter = m_aPropertyMap.find(_nIndex);
            if(aIter == m_aPropertyMap.end())
                sRet = const_cast<OPropertyMap*>(this)->fillValue(_nIndex);
            else
                sRet = aIter->second;
            return sRet;
        }

        ::rtl::OUString OPropertyMap::fillValue(sal_Int32 _nIndex)
        {
            rtl_uString* pStr = NULL;
            switch(_nIndex)
            {
                case PROPERTY_ID_QUERYTIMEOUT:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_QUERYTIMEOUT()         ); break; }
                case PROPERTY_ID_MAXFIELDSIZE:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_MAXFIELDSIZE()         ); break; }
                case PROPERTY_ID_MAXROWS:                   { rtl_uString_newFromAscii(&pStr,getPROPERTY_MAXROWS()              ); break;       }
                case PROPERTY_ID_CURSORNAME:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_CURSORNAME()           ); break;   }
                case PROPERTY_ID_RESULTSETCONCURRENCY:      { rtl_uString_newFromAscii(&pStr,getPROPERTY_RESULTSETCONCURRENCY() ); break; }
                case PROPERTY_ID_RESULTSETTYPE:             { rtl_uString_newFromAscii(&pStr,getPROPERTY_RESULTSETTYPE()            ); break; }
                case PROPERTY_ID_FETCHDIRECTION:            { rtl_uString_newFromAscii(&pStr,getPROPERTY_FETCHDIRECTION()       ); break; }
                case PROPERTY_ID_FETCHSIZE:                 { rtl_uString_newFromAscii(&pStr,getPROPERTY_FETCHSIZE()                ); break;   }
                case PROPERTY_ID_ESCAPEPROCESSING:          { rtl_uString_newFromAscii(&pStr,getPROPERTY_ESCAPEPROCESSING()     ); break; }
                case PROPERTY_ID_USEBOOKMARKS:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_USEBOOKMARKS()         ); break; }
                // Column
                case PROPERTY_ID_NAME:                      { rtl_uString_newFromAscii(&pStr,getPROPERTY_NAME()             ); break; }
                case PROPERTY_ID_TYPE:                      { rtl_uString_newFromAscii(&pStr,getPROPERTY_TYPE()             ); break; }
                case PROPERTY_ID_TYPENAME:                  { rtl_uString_newFromAscii(&pStr,getPROPERTY_TYPENAME()         ); break; }
                case PROPERTY_ID_PRECISION:                 { rtl_uString_newFromAscii(&pStr,getPROPERTY_PRECISION()            ); break; }
                case PROPERTY_ID_SCALE:                     { rtl_uString_newFromAscii(&pStr,getPROPERTY_SCALE()                ); break; }
                case PROPERTY_ID_ISNULLABLE:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISNULLABLE()       ); break; }
                case PROPERTY_ID_ISAUTOINCREMENT:           { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISAUTOINCREMENT()  ); break; }
                case PROPERTY_ID_ISROWVERSION:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISROWVERSION()     ); break; }
                case PROPERTY_ID_DESCRIPTION:               { rtl_uString_newFromAscii(&pStr,getPROPERTY_DESCRIPTION()      ); break; }
                case PROPERTY_ID_DEFAULTVALUE:              { rtl_uString_newFromAscii(&pStr,getPROPERTY_DEFAULTVALUE()     ); break; }

                case PROPERTY_ID_REFERENCEDTABLE:           { rtl_uString_newFromAscii(&pStr,getPROPERTY_REFERENCEDTABLE()  ); break; }
                case PROPERTY_ID_UPDATERULE:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_UPDATERULE()       ); break; }
                case PROPERTY_ID_DELETERULE:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_DELETERULE()       ); break; }
                case PROPERTY_ID_CATALOG:                   { rtl_uString_newFromAscii(&pStr,getPROPERTY_CATALOG()          ); break; }
                case PROPERTY_ID_ISUNIQUE:                  { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISUNIQUE()         ); break; }
                case PROPERTY_ID_ISPRIMARYKEYINDEX:         { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISPRIMARYKEYINDEX()    ); break; }
                case PROPERTY_ID_ISCLUSTERED:               { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISCLUSTERED()          ); break; }
                case PROPERTY_ID_ISASCENDING:               { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISASCENDING()          ); break; }
                case PROPERTY_ID_SCHEMANAME:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_SCHEMANAME()           ); break; }
                case PROPERTY_ID_CATALOGNAME:               { rtl_uString_newFromAscii(&pStr,getPROPERTY_CATALOGNAME()          ); break; }

                case PROPERTY_ID_COMMAND:                   { rtl_uString_newFromAscii(&pStr,getPROPERTY_COMMAND()              ); break; }
                case PROPERTY_ID_CHECKOPTION:               { rtl_uString_newFromAscii(&pStr,getPROPERTY_CHECKOPTION()          ); break; }
                case PROPERTY_ID_PASSWORD:                  { rtl_uString_newFromAscii(&pStr,getPROPERTY_PASSWORD()             ); break; }
                case PROPERTY_ID_RELATEDCOLUMN:             { rtl_uString_newFromAscii(&pStr,getPROPERTY_RELATEDCOLUMN()        ); break;  }

                case PROPERTY_ID_FUNCTION:                  { rtl_uString_newFromAscii(&pStr,getPROPERTY_FUNCTION()             ); break; }
                case PROPERTY_ID_TABLENAME:                 { rtl_uString_newFromAscii(&pStr,getPROPERTY_TABLENAME()                ); break; }
                case PROPERTY_ID_REALNAME:                  { rtl_uString_newFromAscii(&pStr,getPROPERTY_REALNAME()             ); break; }
                case PROPERTY_ID_DBASEPRECISIONCHANGED:     { rtl_uString_newFromAscii(&pStr,getPROPERTY_DBASEPRECISIONCHANGED() ); break; }
                case PROPERTY_ID_ISCURRENCY:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISCURRENCY()           ); break; }
                case PROPERTY_ID_ISBOOKMARKABLE:            { rtl_uString_newFromAscii(&pStr,getPROPERTY_ISBOOKMARKABLE()       ); break; }
                case PROPERTY_ID_INVALID_INDEX:             { rtl_uString_newFromAscii(&pStr,getSTAT_INVALID_INDEX()            ); break; }
                case PROPERTY_ID_ERRORMSG_SEQUENCE:         { rtl_uString_newFromAscii(&pStr,getERRORMSG_SEQUENCE()             ); break; }
                case PROPERTY_ID_HY010:                     { rtl_uString_newFromAscii(&pStr,getSQLSTATE_SEQUENCE()             ); break; }
                case PROPERTY_ID_HY0000:                    { rtl_uString_newFromAscii(&pStr,getSQLSTATE_GENERAL()              ); break; }
                case PROPERTY_ID_DELIMITER:                 { rtl_uString_newFromAscii(&pStr,getSTR_DELIMITER()                 ); break; }
                case PROPERTY_ID_FORMATKEY:                 { rtl_uString_newFromAscii(&pStr,getPROPERTY_FORMATKEY()            ); break; }
                case PROPERTY_ID_LOCALE:                    { rtl_uString_newFromAscii(&pStr,getPROPERTY_LOCALE()               ); break; }
                case PROPERTY_ID_AUTOINCREMENTCREATION:     { rtl_uString_newFromAscii(&pStr,getPROPERTY_AUTOINCREMENTCREATION()        ); break; }
                case PROPERTY_ID_PRIVILEGES:                { rtl_uString_newFromAscii(&pStr,getPROPERTY_PRIVILEGES()           ); break; }
            }
            m_aPropertyMap[_nIndex] = pStr;
            return pStr;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
