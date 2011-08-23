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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLASTPL_IMPL_HXX
#include "impastpl.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include "PageMasterStyleMap.hxx"
#endif
namespace binfilter {

using namespace ::std;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

//#############################################################################
//
// Class SvXMLAutoStylePool_Impl
//

///////////////////////////////////////////////////////////////////////////////
//
// ctor/dtor class SvXMLAutoStylePool_Impl
//

SvXMLAutoStylePoolP_Impl::SvXMLAutoStylePoolP_Impl( SvXMLExport& rExp)
    :	rExport( rExp ),
        maFamilyList( 5, 5 )
{
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
    ULONG nPos;
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

    ULONG nPos;
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

/*OUString SvXMLAutoStylePoolP_Impl::Add( sal_Int32 nFamily,
                                         const OUString& rParent,
                                        const vector< XMLPropertyState >& rProperties,
                                        sal_Bool bCache )*/
sal_Bool SvXMLAutoStylePoolP_Impl::Add(OUString& rName, sal_Int32 nFamily,
                const OUString& rParent,
                const ::std::vector< XMLPropertyState >& rProperties,
                sal_Bool bCache)
{
    sal_Bool bRet(sal_False);
    ULONG nPos;

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

        if( pParent->Add( pFamily, rProperties, rName ) )
        {
            pFamily->mnCount++;
            bRet = sal_True;
        }

        if( bCache )
        {
            if( !pFamily->pCache )
                pFamily->pCache = new SvXMLAutoStylePoolCache_Impl( 256, 256 );
            if( pFamily->pCache->Count() < MAX_CACHE_SIZE )
                pFamily->pCache->Insert( new OUString( rName ),
                                         pFamily->pCache->Count() );
        }
    }

    return bRet;
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

    ULONG nPos;
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
    ULONG nPos;
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
                GetExport().AddAttribute( 
                    XML_NAMESPACE_STYLE, XML_NAME,
                    aExpStyles[i].mpProperties->GetName() );

                if( pFamily->bAsFamily )
                {
                    GetExport().AddAttribute(
                        XML_NAMESPACE_STYLE, XML_FAMILY, aStrFamilyName );
                }

                if( aExpStyles[i].mpParent->getLength() )
                {
                    GetExport().AddAttribute(
                        XML_NAMESPACE_STYLE, XML_PARENT_STYLE_NAME,
                        *aExpStyles[i].mpParent );
                }

                OUString sName;
                if( pFamily->bAsFamily )
                    sName = GetXMLToken(XML_STYLE);
                else
                    sName = pFamily->maStrFamilyName;

                pAntiImpl->exportStyleAttributes(
                    GetExport().GetAttrList(),
                    nFamily,
                    aExpStyles[i].mpProperties->GetProperties(),
                    *pFamily->mxMapper.get()
                        , GetExport().GetMM100UnitConverter(),
                        GetExport().GetNamespaceMap()
                    );

                SvXMLElementExport aElem( GetExport(), 
                                          XML_NAMESPACE_STYLE, sName,
                                          sal_True, sal_True );

                sal_Int32 nStart(-1);
                sal_Int32 nEnd(-1);
                if (nFamily == XML_STYLE_FAMILY_PAGE_MASTER)
                {
                    nStart = 0;
                    sal_Int32 nIndex = 0;
                    UniReference< XMLPropertySetMapper > aPropMapper =
                        pFamily->mxMapper->getPropertySetMapper();
                    sal_Int16 nContextID;
                    while(nIndex < aPropMapper->GetEntryCount() && nEnd == -1)
                    {
                        nContextID = aPropMapper->GetEntryContextId( nIndex );
                        if (nContextID && ((nContextID & CTF_PM_FLAGMASK) != XML_PM_CTF_START))
                            nEnd = nIndex;
                        nIndex++;
                    }
                    if (nEnd == -1)
                        nEnd = nIndex;
                }

                pFamily->mxMapper->exportXML(
                    GetExport(),
                    aExpStyles[i].mpProperties->GetProperties(), 
                    nStart, nEnd, XML_EXPORT_FLAG_IGN_WS );

                pAntiImpl->exportStyleContent( 
                    GetExport().GetDocHandler(),
                    nFamily,
                    aExpStyles[i].mpProperties->GetProperties(),
                    *pFamily->mxMapper.get(), 
                    GetExport().GetMM100UnitConverter(),
                    GetExport().GetNamespaceMap()
                    );
            }
        }

        delete[] aExpStyles;
    }
}

void SvXMLAutoStylePoolP_Impl::ClearEntries()
{
    for(sal_uInt32 a = 0L; a < maFamilyList.Count(); a++)
        maFamilyList[a]->ClearEntries();
}
}//end of namespace binfilter
