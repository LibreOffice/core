/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impimagetree.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:00:11 $
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

#ifndef _SV_IMPIMAGETREE_HXX
#define _SV_IMPIMAGETREE_HXX

#include <memory>

#ifndef _COMPHELPER_SINGLETONREF_HXX_
#include <salhelper/singletonref.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

// ----------------
// -ImplImageTree -
// ----------------

class BitmapEx;
namespace com { namespace sun { namespace star { namespace packages { namespace zip { class XZipFileAccess; } } } } }
namespace com { namespace sun { namespace star { namespace container { class XNameAccess; } } } }
namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace ucb { class XSimpleFileAccess; } } } }
namespace com { namespace sun { namespace star { namespace io { class XInputStream; } } } }

class ImplImageTree
{
public:

                    ImplImageTree();
                    ~ImplImageTree();

    bool            loadImage( const ::rtl::OUString& rName,
                               const ::rtl::OUString& rSymbolsStyle,
                               BitmapEx& rReturn,
                               bool bSearchLanguageDependent = false );
    void            addUserImage( const ::rtl::OUString& rName, const BitmapEx& rReturn );

    static void     cleanup();

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::packages::zip::XZipFileAccess > mxZipAcc;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        mxNameAcc;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >           mxPathSettings;
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >        mxFileAccess;
    bool                                                                                mbInit;
    ::rtl::OUString                                                                     maSymbolsStyle;

    bool                    implInit();
    ::rtl::OUString         implGetZipFileURL( bool bWithStyle = true ) const;
    const ::rtl::OUString&  implGetUserDirURL() const;
    ::rtl::OUString         implGetUserFileURL( const ::rtl::OUString& rName ) const;
    void                    implCheckUserCache();
    bool                    implLoadFromStream( SvStream& rIStm, const ::rtl::OUString& rFileName, BitmapEx& rReturn );
    ::std::auto_ptr< SvStream > implGetStream( const ::com::sun::star::uno::Reference<
                                                        ::com::sun::star::io::XInputStream >& rxIStm ) const;
    void                    implUpdateSymbolsStyle( const ::rtl::OUString& rSymbolsStyle );
};

typedef ::salhelper::SingletonRef< ImplImageTree > ImplImageTreeSingletonRef;

#endif // _SV_IMPIMAGETREE_HXX
