/*************************************************************************
 *
 *  $RCSfile: cr_html.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: np $ $Date: 2001-03-23 13:24:04 $
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

#ifndef X2C_CR_HTML_HXX
#define X2C_CR_HTML_HXX


#include <string.h>
#include <fstream.h>
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
                            char *              i_pText );
    void                StartRow();
    void                FinishRow();
    void                StartCell(
                            char *              i_pWidth );
    void                FinishCell();

    void                WriteElementName(
                            const Simstr &      i_sName,
                            bool                i_bStrong );
    void                WriteStr(
                            const char *        i_sStr )
                                                { aFile.write( i_sStr, strlen(i_sStr) ); }
    // DATA
    ofstream            aFile;
    const XmlElement &  rDocument;
    Simstr              sIdl_BaseDirectory;
};




#endif
