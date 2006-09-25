/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cr_html.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2006-09-25 13:26:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef X2C_CR_HTML_HXX
#define X2C_CR_HTML_HXX


#include <string.h>
#include <fstream>
#include "../support/sistr.hxx"
#include "../support/syshelp.hxx"


class XmlElement;
class SglTextElement;
class MultipleTextElement;


class HtmlCreator
{
  public:
                        HtmlCreator(
                            const char *        i_pOutputFileName,
                            const XmlElement &  i_rDocument,
                            const Simstr &      i_sIDL_BaseDirectory );
                        ~HtmlCreator();

    void                Run();

    void                StartTable();
    void                FinishTable();
    void                StartBigCell(
                            const char *        i_sTitle );
    void                FinishBigCell();

    void                Write_SglTextElement(
                            const SglTextElement &
                                                i_rElement,
                            bool                i_bStrong = false );
    void                Write_MultiTextElement(
                            const MultipleTextElement &
                                                i_rElement );
    void                Write_SglText(
                            const Simstr &      i_sName,
                            const Simstr &      i_sValue );
    void                Write_ReferenceDocu(
                            const Simstr &      i_sName,
                            const Simstr &      i_sRef,
                            const Simstr &      i_sRole,
                            const Simstr &      i_sTitle );
  private:
    void                PrintH1(
                            const char *        i_pText );
    void                StartRow();
    void                FinishRow();
    void                StartCell(
                            const char *        i_pWidth );
    void                FinishCell();

    void                WriteElementName(
                            const Simstr &      i_sName,
                            bool                i_bStrong );
    void                WriteStr(
                            const char *        i_sStr )
                                                { aFile.write( i_sStr, (int) strlen(i_sStr) ); }
    // DATA
    std::ofstream       aFile;
    const XmlElement &  rDocument;
    Simstr              sIdl_BaseDirectory;
};




#endif


