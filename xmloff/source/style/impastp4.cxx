/*************************************************************************
 *
 *  $RCSfile: impastp4.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-07 13:33:06 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLASTPLP_HXX
#include "xmlaustp.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif
#ifndef _XMLOFF_XMLASTPL_IMPL_HXX
#include "impastpl.hxx"
#endif
#ifndef _XMLOFF_XMLEXPPR_HXX
#include "xmlexppr.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif
#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include "PageMasterStyleMap.hxx"
#endif

using namespace ::std;
using namespace ::rtl;
using namespace ::com::sun::star;

//#############################################################################
//
// Class SvXMLAutoStylePool_Impl
//

///////////////////////////////////////////////////////////////////////////////
//
// ctor/dtor class SvXMLAutoStylePool_Impl
//

SvXMLAutoStylePoolP_Impl::SvXMLAutoStylePoolP_Impl()
:   msCDATA( OUString::createFromAscii( sXML_CDATA ) ),
    msWS( OUString::createFromAscii( sXML_WS ) ),
    maFamilyList( 5, 5 )
{
    mpAttrList = new SvXMLAttributeList();
    mxAttrList = mpAttrList;
}

SvXMLAutoStylePoolP_Impl::~SvXMLAutoStylePoolP_Impl()
{
    XMLFamilyData_Impl* pData = NULL;
    while( ( pData = maFamilyList.Remove( ULONG(0) ) ) )
        delete pData;
}

///////////////////////////////////////////////////////////////////////////////
//
// Adds stylefamily-informations to sorted list
//

void SvXMLAutoStylePoolP_Impl::AddFamily(
        sal_Int32 nFamily,
        const OUString& rStrName,
        const UniReference < SvXMLExportPropertyMapper > & rMapper,
           const OUString& rStrPrefix,
        sal_Bool bAsFamily )
{
    // store family in a list if not already stored
    sal_uInt32 nPos;
    XMLFamilyData_Impl *pFamily = new XMLFamilyData_Impl( nFamily, rStrName, rMapper, rStrPrefix, bAsFamily );
    if( !maFamilyList.Seek_Entry( pFamily, &nPos ) )
        maFamilyList.Insert( pFamily );
    else
        delete pFamily;
}

///////////////////////////////////////////////////////////////////////////////
//
// Adds a name to list
//

void SvXMLAutoStylePoolP_Impl::RegisterName( sal_Int32 nFamily, const OUString& rName )
{
    SvXMLAutoStylePoolNamesP_Impl *pNames = 0;

    sal_uInt32 nPos;
    XMLFamilyData_Impl aTmp( nFamily );
    if( maFamilyList.Seek_Entry( &aTmp, &nPos ) )
        pNames = maFamilyList.GetObject( nPos )->mpNameList;

    DBG_ASSERT( pNames,
                "SvXMLAutoStylePool_Impl::RegisterName: unknown family" );
    if( pNames )
    {
        OUString *pName = new OUString( rName );
        if( !pNames->Insert( pName ) )
            delete pName;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// Adds a array of XMLPropertyState ( vector< XMLPropertyState > ) to list
// if not added, yet.
//

OUString SvXMLAutoStylePoolP_Impl::Add( sal_Int32 nFamily,
                                         const OUString& rParent,
                                        const vector< XMLPropertyState >& rProperties )
{
    OUString sName;
    sal_uInt32 nPos;

    XMLFamilyData_Impl *pFamily = 0;
    XMLFamilyData_Impl aTmp( nFamily );
    if( maFamilyList.Seek_Entry( &aTmp, &nPos ) )
    {
        pFamily = maFamilyList.GetObject( nPos );
    }

    DBG_ASSERT( pFamily, "SvXMLAutoStylePool_Impl::Add: unknown family" );
    if( pFamily )
    {
        SvXMLAutoStylePoolParentP_Impl aTmp( rParent );
        SvXMLAutoStylePoolParentP_Impl *pParent = 0;

        SvXMLAutoStylePoolParentsP_Impl *pParents = pFamily->mpParentList;
        if( pParents->Seek_Entry( &aTmp, &nPos ) )
        {
            pParent = pParents->GetObject( nPos );
        }
        else
        {
            pParent = new SvXMLAutoStylePoolParentP_Impl( rParent );
            pParents->Insert( pParent );
        }

        if( pParent->Add( pFamily, rProperties, sName ) )
            pFamily->mnCount++;
    }

    return sName;
}

///////////////////////////////////////////////////////////////////////////////
//
// Search for a array of XMLPropertyState ( vector< XMLPropertyState > ) in list
//

OUString SvXMLAutoStylePoolP_Impl::Find( sal_Int32 nFamily,
                                         const OUString& rParent,
                                         const vector< XMLPropertyState >& rProperties ) const
{
    OUString sName;

    sal_uInt32 nPos;
    XMLFamilyData_Impl aTmp( nFamily );
    XMLFamilyData_Impl *pFamily = 0;
    if( maFamilyList.Seek_Entry( &aTmp, &nPos ) )
    {
        pFamily = maFamilyList.GetObject( nPos );
    }

    DBG_ASSERT( pFamily, "SvXMLAutoStylePool_Impl::Find: unknown family" );

    if( pFamily )
    {
        SvXMLAutoStylePoolParentP_Impl aTmp( rParent );

        const SvXMLAutoStylePoolParentsP_Impl* pParents =
            pFamily->mpParentList;
        if( pParents->Seek_Entry( &aTmp, &nPos ) )
            sName = pParents->GetObject( nPos )->Find( pFamily, rProperties );
    }

    return sName;
}

///////////////////////////////////////////////////////////////////////////////
//
// export
//

void SvXMLAutoStylePoolP_Impl::exportXML(
           sal_Int32 nFamily,
        const uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const SvXMLAutoStylePoolP *pAntiImpl) const
{
    sal_uInt32 nCount = 0;

    // Get list of parents for current family (nFamily)
    sal_uInt32 nPos;
    XMLFamilyData_Impl aTmp( nFamily );
    XMLFamilyData_Impl *pFamily = 0;
    if( maFamilyList.Seek_Entry( &aTmp, &nPos ) )
    {
        pFamily = maFamilyList.GetObject( nPos );
        nCount         = pFamily->mnCount;
    }

    DBG_ASSERT( pFamily,
                "SvXMLAutoStylePool_Impl::exportXML: unknown family" );
    if( pFamily && nCount > 0 )
    {
        /////////////////////////////////////////////////////////////////////////////////////
        // create, initialize and fill helper-structure (SvXMLAutoStylePoolProperties_Impl)
        // wich contains a parent-name and a SvXMLAutoStylePoolProperties_Impl
        //
        const SvXMLAutoStylePoolParentsP_Impl *pParents =
            pFamily->mpParentList;

        SvXMLAutoStylePoolPExport_Impl* aExpStyles =
            new SvXMLAutoStylePoolPExport_Impl[nCount];

        sal_uInt32 i;
        for( i=0; i < nCount; i++ )
        {
            aExpStyles[i].mpParent = 0;
            aExpStyles[i].mpProperties = 0;
        }

        sal_uInt32 nParents = pParents->Count();
        for( i=0; i < nParents; i++ )
        {
            const SvXMLAutoStylePoolParentP_Impl* pParent =
                pParents->GetObject( i );
            sal_uInt32 nProperties = pParent->GetPropertiesList().Count();
            for( sal_uInt32 j=0; j < nProperties; j++ )
            {
                const SvXMLAutoStylePoolPropertiesP_Impl *pProperties =
                    pParent->GetPropertiesList().GetObject( j );
                sal_uInt32 nPos = pProperties->GetPos();
                DBG_ASSERT( nPos < nCount,
                        "SvXMLAutoStylePool_Impl::exportXML: wrong position" );
                if( nPos < nCount )
                {
                    DBG_ASSERT( !aExpStyles[nPos].mpProperties,
                        "SvXMLAutoStylePool_Impl::exportXML: double position" );
                    aExpStyles[nPos].mpProperties = pProperties;
                    aExpStyles[nPos].mpParent = &pParent->GetParent();
                }
            }
        }

        /////////////////////////////////////////////////////////////////////////////////////
        //
        // create string to export for each XML-style. That means for each property-list
        //
        OUString aStrFamilyName = pFamily->maStrFamilyName;

        for( i=0; i<nCount; i++ )
        {
            DBG_ASSERT( aExpStyles[i].mpProperties,
                        "SvXMLAutoStylePool_Impl::exportXML: empty position" );

            if( aExpStyles[i].mpProperties )
            {
                OUString sName( rNamespaceMap.GetQNameByKey( XML_NAMESPACE_STYLE, OUString::createFromAscii(sXML_name) ) );
                mpAttrList->AddAttribute( sName, msCDATA, aExpStyles[i].mpProperties->GetName() );

                if( pFamily->bAsFamily )
                {
                    sName = rNamespaceMap.GetQNameByKey( XML_NAMESPACE_STYLE, OUString::createFromAscii(sXML_family) );
                    mpAttrList->AddAttribute( sName, msCDATA, aStrFamilyName );
                }

                if( aExpStyles[i].mpParent->getLength() )
                {
                    sName = rNamespaceMap.GetQNameByKey( XML_NAMESPACE_STYLE, OUString::createFromAscii(sXML_parent_style_name) );
                    mpAttrList->AddAttribute( sName, msCDATA, *aExpStyles[i].mpParent );
                }

                if( pFamily->bAsFamily )
                    sName = rNamespaceMap.GetQNameByKey( XML_NAMESPACE_STYLE, OUString::createFromAscii(sXML_style) );
                else
                    sName = rNamespaceMap.GetQNameByKey( XML_NAMESPACE_STYLE, pFamily->maStrFamilyName );

                pAntiImpl->exportStyleAttributes( *mpAttrList, nFamily,
                        aExpStyles[i].mpProperties->GetProperties(),
                        *pFamily->mxMapper.get(),
                        rUnitConverter, rNamespaceMap );

                rHandler->ignorableWhitespace( msWS );
                rHandler->startElement( sName, mxAttrList );
                mpAttrList->Clear();
                sal_Int32 nStart(-1);
                sal_Int32 nEnd(-1);
                if (nFamily == XML_STYLE_FAMILY_PAGE_MASTER)
                {
                    nStart = 0;
                    sal_Int32 nIndex = 0;
                    UniReference< XMLPropertySetMapper > aPropMapper =
                        pFamily->mxMapper->getPropertySetMapper();
                    while(nIndex < aPropMapper->GetEntryCount() && nEnd == -1)
                    {
                        if (aPropMapper->GetEntryContextId( nIndex ) & CTF_PM_FLAGMASK)
                            nEnd = nIndex;
                        nIndex++;
                    }
                    if (nEnd == -1)
                        nEnd = nIndex;
                }

                pFamily->mxMapper->exportXML( rHandler, aExpStyles[i].mpProperties->GetProperties(), rUnitConverter,
                                    rNamespaceMap, nStart, nEnd, XML_EXPORT_FLAG_IGN_WS );

                pAntiImpl->exportStyleContent( rHandler, nFamily,
                        aExpStyles[i].mpProperties->GetProperties(),
                        *pFamily->mxMapper.get(), rUnitConverter, rNamespaceMap );

                rHandler->ignorableWhitespace( msWS );
                rHandler->endElement( sName );
            }
        }

        delete aExpStyles;
    }
}

void SvXMLAutoStylePoolP_Impl::ClearEntries()
{
    for(sal_uInt32 a = 0L; a < maFamilyList.Count(); a++)
        maFamilyList[a]->ClearEntries();
}
