/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impimagetree.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_IMPIMAGETREE_HXX
#define _SV_IMPIMAGETREE_HXX

#include <memory>

#ifndef _COMPHELPER_SINGLETONREF_HXX_
#include <salhelper/singletonref.hxx>
#endif
#include <com/sun/star/uno/Reference.hxx>

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
