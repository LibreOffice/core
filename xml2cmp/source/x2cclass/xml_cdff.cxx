/*************************************************************************
 *
 *  $RCSfile: xml_cdff.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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



