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

#include "xml_cdff.hxx"

#include <string.h>
#include <tools/stream.hxx>
#include "xml_cdim.hxx"
#include <ctype.h>


typedef ComponentDescriptionImpl::ValueList CdiValueList;

class dyn_buffer
{
  public:
                        dyn_buffer()                    : s(0) {}
                        ~dyn_buffer()                   { if (s) delete [] s; }
                        operator const char *() const   { return s; }
    char *              operator->()                    { return s; }
    char &              operator[](
                            INT32 ix )                  { return s[ix]; }
    void                SetSize(
                            INT32 i_size )              { if (s) delete [] s; s = new char [i_size]; }
  private:
    char * s;
};


inline BOOL
LoadXmlFile( dyn_buffer &       o_rBuffer,
             const UniString &  i_sXmlFilePath )
{
    BOOL ret = TRUE;
    SvFileStream aXmlFile;

    aXmlFile.Open(i_sXmlFilePath, STREAM_READ);
    if (aXmlFile.GetErrorCode() != FSYS_ERR_OK)
        ret = FALSE;
    if (ret)
    {
        aXmlFile.Seek(STREAM_SEEK_TO_END);
        INT32 nBufferSize = aXmlFile.Tell();
        o_rBuffer.SetSize(nBufferSize + 1);
        o_rBuffer[nBufferSize] = '\0';
        aXmlFile.Seek(0);
        if (aXmlFile.Read(o_rBuffer.operator->(), nBufferSize) == 0)
            ret = FALSE;
    }

    aXmlFile.Close();
    return ret;
}



CompDescrsFromAnXmlFile::CompDescrsFromAnXmlFile()
    :   dpDescriptions(new std::vector< ComponentDescriptionImpl* >),
        eStatus(not_yet_parsed)
{
    dpDescriptions->reserve(3);
}

CompDescrsFromAnXmlFile::~CompDescrsFromAnXmlFile()
{
    Empty();
    delete dpDescriptions;
}


BOOL
CompDescrsFromAnXmlFile::Parse( const UniString & i_sXmlFilePath )
{
    dyn_buffer      dpBuffer;

    if (! LoadXmlFile(dpBuffer,i_sXmlFilePath) )
    {
        eStatus = cant_read_file;
        return FALSE;
    }

    const char *    pTokenStart = 0;
    const char *    pBufferPosition = dpBuffer;
    INT32           nTokenLength = 0;
    BOOL            bWithinElement = FALSE;

    CdiValueList *  pCurTagData = 0;
    ByteString      sStatusValue;   // Used only if a <Status ...> tag is found.


    for ( ComponentDescriptionImpl::ParseUntilStartOfDescription(pBufferPosition);
          pBufferPosition != 0;
          ComponentDescriptionImpl::ParseUntilStartOfDescription(pBufferPosition) )
    {
        ComponentDescriptionImpl * pCurCD = 0;
        pCurCD = new ComponentDescriptionImpl;
        dpDescriptions->push_back(pCurCD);

        for ( ; *pBufferPosition != '\0' && pCurCD != 0; )
        {
            switch (*pBufferPosition)
            {
                case '<' :
                        if (! bWithinElement)
                        {
                            pCurTagData = pCurCD->GetBeginTag(sStatusValue, pBufferPosition);
                            if (pCurTagData != 0)
                            {
                                if (sStatusValue.Len () == 0)
                                {
                                    // Start new token:
                                    pTokenStart = pBufferPosition;
                                    nTokenLength = 0;
                                    bWithinElement = TRUE;;
                                }
                                else
                                {
                                    // Status tag is already parsed:
                                    pCurTagData->push_back(sStatusValue);
                                }   // endif (sStatusValue.Length () == 0)
                            }
                            else if ( ComponentDescriptionImpl::CheckEndOfDescription(pBufferPosition) )
                            {
                                pBufferPosition += ComponentDescriptionImpl::DescriptionEndTagSize();
                                pCurCD = 0;
                            }
                            else
                            {
                                eStatus = inconsistent_file;
                                return FALSE;
                            }   // endif (pCurTagData != 0) elseif() else
                        }
                        else if ( pCurTagData->MatchesEndTag(pBufferPosition) )
                        {
                            // Finish token:
                            pBufferPosition += pCurTagData->EndTagLength();
                            bWithinElement = FALSE;

                                // Remove leading and trailing spaces:
                            while ( isspace(*pTokenStart) )
                            {
                               pTokenStart++;
                               nTokenLength--;
                            }
                            while ( nTokenLength > 0
                                    && isspace(pTokenStart[nTokenLength-1]) )
                            {
                                nTokenLength--;
                            }
                                // Add token to tag values list.
                            pCurTagData->push_back(ByteString(pTokenStart,nTokenLength));
                        }
                        else
                        {
                            nTokenLength++;
                            ++pBufferPosition;
                        }   // endif (!bWithinElement) else if () else
                       break;
                default:
                        if (bWithinElement)
                        {
                            ++nTokenLength;
                        }
                        ++pBufferPosition;
            }   // end switch
        }   // end for

        if (bWithinElement)
        {
            eStatus = inconsistent_file;
            return FALSE;
        }
    } // end for

    return TRUE;
}

INT32
CompDescrsFromAnXmlFile::NrOfDescriptions() const
{
    return dpDescriptions->size();
}

const ComponentDescription &
CompDescrsFromAnXmlFile::operator[](INT32 i_nIndex) const
{
    static const ComponentDescriptionImpl aNullDescr_;
    return 0 <= i_nIndex && i_nIndex < dpDescriptions->size()
                ?   *(*dpDescriptions)[i_nIndex]
                :   aNullDescr_;
}

void
CompDescrsFromAnXmlFile::Empty()
{
    for ( std::vector< ComponentDescriptionImpl* >::iterator aIter = dpDescriptions->begin();
          aIter != dpDescriptions->end();
          ++aIter )
    {
        delete *aIter;
    }
    dpDescriptions->erase( dpDescriptions->begin(),
                           dpDescriptions->end() );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
