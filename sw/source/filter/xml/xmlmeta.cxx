/*************************************************************************
 *
 *  $RCSfile: xmlmeta.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:15:00 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include <hintids.hxx>

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif

#ifndef _XMLOFF_XMLMETAI_HXX
#include <xmloff/xmlmetai.hxx>
#endif

#ifndef _XMLOFF_XMLMETAE_HXX
#include <xmloff/xmlmetae.hxx>
#endif

#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif

#ifndef _SWDOCSH_HXX
#include "docsh.hxx"
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif

#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLEXP_HXX
#include "xmlexp.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

// ---------------------------------------------------------------------

#if 0
class SwXMLMetaContext_Impl : public SfxXMLMetaContext
{
public:
    SwXMLMetaContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< document::XDocumentInfo > & rInfo ) :
        SfxXMLMetaContext( rImport, nPrfx, rLName, rInfo )
    {}

    virtual void SetDocLanguage( LanguageType eLang );
};

void SwXMLMetaContext_Impl::SetDocLanguage( LanguageType eLang )
{
    SwDoc& rDoc = ((SwXMLImport&)GetImport()).GetDoc();

    rDoc.SetDefault( SvxLanguageItem( eLang, RES_CHRATR_LANGUAGE ) );
}
#endif

// ---------------------------------------------------------------------

SvXMLImportContext *SwXMLImport::CreateMetaContext(
                                       const OUString& rLocalName )
{
    SvXMLImportContext *pContext = 0;

    if( !(IsStylesOnlyMode() || IsInsertMode()) )
    {
        pContext = new SfxXMLMetaContext( *this,
                                    XML_NAMESPACE_OFFICE, rLocalName,
                                    GetModel() );
#if 0
        SfxObjectShell* pObjSh = pDoc->GetDocShell();
        if( pObjSh )
        {
            Reference< frame::XModel >  xModel = pObjSh->GetBaseModel();
            Reference< document::XDocumentInfoSupplier > xSupp( xModel,
                                                                UNO_QUERY );
            if( xSupp.is() )
                pContext = new SwXMLMetaContext_Impl( *this,
                                    XML_NAMESPACE_OFFICE, rLocalName,
                                    xSupp->getDocumentInfo() );
        }
#endif
    }

    if( !pContext )
        pContext = new SvXMLImportContext( *this, XML_NAMESPACE_OFFICE,
                                              rLocalName );

    return pContext;
}

// ---------------------------------------------------------------------

#if 0
void SwXMLExport::_ExportMeta()
{
    SfxObjectShell* pObjSh = pDoc->GetDocShell();
    if( pObjSh )
    {
        pObjSh->UpdateDocInfoForSave();     // update information

        LanguageType eDocLang = ((const SvxLanguageItem&)
                pDoc->GetDefault(RES_CHRATR_LANGUAGE)).GetLanguage();

        Reference< frame::XModel >  xModel = pObjSh->GetBaseModel();
        Reference< document::XDocumentInfoSupplier >  xSupp( xModel,
                                                            UNO_QUERY );
        if( xSupp.is() )
        {
            SfxXMLMetaExport aMeta( GetDocHandler(), xSupp->getDocumentInfo(),
                                    eDocLang );
            aMeta.Export( GetNamespaceMap() );
        }
    }
}
#endif


/*************************************************************************

      Source Code Control chaos::System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/xml/xmlmeta.cxx,v 1.1.1.1 2000-09-18 17:15:00 hr Exp $

      Source Code Control chaos::System - Update

      $Log: not supported by cvs2svn $
      Revision 1.7  2000/09/18 16:05:07  willem.vandorp
      OpenOffice header added.

      Revision 1.6  2000/08/02 14:52:39  mib
      text export continued

      Revision 1.5  2000/06/08 09:45:54  aw
      changed to use functionality from xmloff project now

      Revision 1.4  2000/05/03 12:08:05  mib
      unicode

      Revision 1.3  2000/03/13 14:33:44  mib
      UNO3

      Revision 1.2  2000/02/11 14:42:04  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.1  2000/01/06 15:03:49  mib
      #70271#:separation of text/layout, cond. styles, adaptions to wd-xlink-19991229


*************************************************************************/

