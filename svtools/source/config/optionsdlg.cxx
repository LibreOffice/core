/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optionsdlg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-14 12:59:32 $
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

#include "optionsdlg.hxx"

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#include <hash_map>
#include "itemholder1.hxx"

using namespace utl;
using namespace rtl;
using namespace com::sun::star::beans ;
using namespace com::sun::star::uno;

#define CFG_FILENAME            OUString( RTL_CONSTASCII_USTRINGPARAM( "Office.OptionsDialog" ) )
#define ROOT_NODE               OUString( RTL_CONSTASCII_USTRINGPARAM( "OptionsDialogGroups" ) )
#define PAGES_NODE              OUString( RTL_CONSTASCII_USTRINGPARAM( "Pages" ) )
#define OPTIONS_NODE            OUString( RTL_CONSTASCII_USTRINGPARAM( "Options" ) )
#define PROPERTY_HIDE           OUString( RTL_CONSTASCII_USTRINGPARAM( "Hide" ) )

static SvtOptionsDlgOptions_Impl*   pOptions = NULL;
static sal_Int32                    nRefCount = 0;

class SvtOptionsDlgOptions_Impl : public utl::ConfigItem
{
private:
    struct OUStringHashCode
    {
        size_t operator()( const ::rtl::OUString& sString ) const
        {
            return sString.hashCode();
        }
    };

    typedef std::hash_map< OUString, sal_Bool, OUStringHashCode, ::std::equal_to< OUString > > OptionNodeList;

    OUString        m_sPathDelimiter;
    OptionNodeList  m_aOptionNodeList;

    enum NodeType{ NT_Group, NT_Page, NT_Option };
    void            ReadNode( const OUString& _rNode, NodeType _eType );
    sal_Bool        IsHidden( const OUString& _rPath ) const;

public:
                    SvtOptionsDlgOptions_Impl();

    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void    Commit();

    static ::osl::Mutex & getInitMutex();

    sal_Bool        IsGroupHidden   (   const OUString& _rGroup ) const;
    sal_Bool        IsPageHidden    (   const OUString& _rPage,
                                        const OUString& _rGroup ) const;
    sal_Bool        IsOptionHidden  (   const OUString& _rOption,
                                        const OUString& _rPage,
                                        const OUString& _rGroup ) const;
};

::osl::Mutex & SvtOptionsDlgOptions_Impl::getInitMutex()
{
    static ::osl::Mutex *pMutex = 0;

    if( ! pMutex )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( ! pMutex )
        {
            static ::osl::Mutex mutex;
            pMutex = &mutex;
        }
    }
    return *pMutex;
}

// -----------------------------------------------------------------------

SvtOptionsDlgOptions_Impl::SvtOptionsDlgOptions_Impl()
    : ConfigItem( OUString( CFG_FILENAME ) ),

    m_sPathDelimiter( RTL_CONSTASCII_USTRINGPARAM( "/" ) ),
    m_aOptionNodeList( OptionNodeList() )

{
    OUString sRootNode( ROOT_NODE );
    Sequence< OUString > aNodeSeq = GetNodeNames( sRootNode );
    OUString sNode( sRootNode + m_sPathDelimiter );
    sal_uInt32 nCount = aNodeSeq.getLength();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString sSubNode( sNode + aNodeSeq[n] );
        ReadNode( sSubNode, NT_Group );
    }
}

// -----------------------------------------------------------------------

void SvtOptionsDlgOptions_Impl::Commit()
{
    // nothing to commit
}

// -----------------------------------------------------------------------

void SvtOptionsDlgOptions_Impl::Notify( const Sequence< rtl::OUString >& )
{
    // nothing to notify
}

void SvtOptionsDlgOptions_Impl::ReadNode( const OUString& _rNode, NodeType _eType )
{
    OUString sNode( _rNode + m_sPathDelimiter );
    OUString sSet;
    sal_Int32 nLen = 0;
    switch ( _eType )
    {
        case NT_Group :
        {
            sSet = PAGES_NODE;
            nLen = 2;
            break;
        }

        case NT_Page :
        {
            sSet = OPTIONS_NODE;
            nLen = 2;
            break;
        }

        case NT_Option :
        {
            nLen = 1;
            break;
        }
    }

    Sequence< OUString > lResult( nLen );
    lResult[0] = OUString( sNode + PROPERTY_HIDE );
    if ( _eType != NT_Option )
        lResult[1] = OUString( sNode + sSet );

    Sequence< Any > aValues;
    aValues = GetProperties( lResult );
    sal_Bool bHide = sal_False;
    if ( aValues[0] >>= bHide )
        m_aOptionNodeList.insert( OptionNodeList::value_type( sNode, bHide ) );

    if ( _eType != NT_Option )
    {
        OUString sNodes( sNode + sSet );
        Sequence< OUString > aNodes = GetNodeNames( sNodes );
        if ( aNodes.getLength() > 0 )
        {
            for ( sal_uInt32 n = 0; n < (sal_uInt32)aNodes.getLength(); ++n )
            {
                OUString sSubNodeName( sNodes + m_sPathDelimiter + aNodes[n] );
                ReadNode( sSubNodeName, _eType == NT_Group ? NT_Page : NT_Option );
            }
        }
    }
}

// -----------------------------------------------------------------------

OUString getGroupPath( const OUString& _rGroup )
{
    return OUString( ROOT_NODE + OUString('/') + _rGroup + OUString('/') );
}
OUString getPagePath( const OUString& _rPage )
{
    return OUString( PAGES_NODE + OUString('/') + _rPage + OUString('/') );
}
OUString getOptionPath( const OUString& _rOption )
{
    return OUString( OPTIONS_NODE + OUString('/') + _rOption + OUString('/') );
}

// -----------------------------------------------------------------------

sal_Bool SvtOptionsDlgOptions_Impl::IsHidden( const OUString& _rPath ) const
{
    sal_Bool bRet = sal_False;
    OptionNodeList::const_iterator pIter = m_aOptionNodeList.find( _rPath );
    if ( pIter != m_aOptionNodeList.end() )
        bRet = pIter->second;
    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SvtOptionsDlgOptions_Impl::IsGroupHidden( const OUString& _rGroup ) const
{
    return IsHidden( getGroupPath( _rGroup ) );
}

// -----------------------------------------------------------------------

sal_Bool SvtOptionsDlgOptions_Impl::IsPageHidden( const OUString& _rPage, const OUString& _rGroup ) const
{
    return IsHidden( getGroupPath( _rGroup  ) + getPagePath( _rPage ) );
}

// -----------------------------------------------------------------------

sal_Bool SvtOptionsDlgOptions_Impl::IsOptionHidden(
    const OUString& _rOption, const OUString& _rPage, const OUString& _rGroup ) const
{
    return IsHidden( getGroupPath( _rGroup  ) + getPagePath( _rPage ) + getOptionPath( _rOption ) );
}

// -----------------------------------------------------------------------

SvtOptionsDialogOptions::SvtOptionsDialogOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( SvtOptionsDlgOptions_Impl::getInitMutex() );
    ++nRefCount;
    if ( !pOptions )
    {
        pOptions = new SvtOptionsDlgOptions_Impl;

        ItemHolder1::holdConfigItem( E_OPTIONSDLGOPTIONS );
    }
    m_pImp = pOptions;
}

// -----------------------------------------------------------------------

SvtOptionsDialogOptions::~SvtOptionsDialogOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( SvtOptionsDlgOptions_Impl::getInitMutex() );
    if ( !--nRefCount )
    {
        if ( pOptions->IsModified() )
            pOptions->Commit();
        DELETEZ( pOptions );
    }
}

sal_Bool SvtOptionsDialogOptions::IsGroupHidden( const String& _rGroup ) const
{
    return m_pImp->IsGroupHidden( _rGroup );
}

sal_Bool SvtOptionsDialogOptions::IsPageHidden( const String& _rPage, const String& _rGroup ) const
{
    return m_pImp->IsPageHidden( _rPage, _rGroup );
}

sal_Bool SvtOptionsDialogOptions::IsOptionHidden(
    const String& _rOption, const String& _rPage, const String& _rGroup ) const
{
    return m_pImp->IsOptionHidden( _rOption, _rPage, _rGroup );
}

