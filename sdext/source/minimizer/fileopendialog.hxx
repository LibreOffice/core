/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fileopendialog.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sj $ $Date: 2007-05-11 13:50:28 $
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


#ifndef _FILEOPEN_DIALOG_HXX_
#define _FILEOPEN_DIALOG_HXX_

#include <vector>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_STRINGPAIR_HPP_
#include <com/sun/star/beans/StringPair.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif

class FileOpenDialog
{
    // a list of filters that are provided within the SaveDialog
    struct FilterEntry
    {
        rtl::OUString maName;
        rtl::OUString maType;
        rtl::OUString maUIName;
        rtl::OUString maFilter;
        sal_Int32 maFlags;
        com::sun::star::uno::Sequence< rtl::OUString > maExtensions;

        FilterEntry() :
            maFlags( 0 ) {}
    };
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxMSF;
    com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XFilePicker > mxFilePicker;

    std::vector< FilterEntry > aFilterEntryList;

public :
        FileOpenDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF );
        ~FileOpenDialog();

        sal_Int16 execute();

        void setDefaultName( const rtl::OUString& );
        void setDefaultDirectory( const rtl::OUString& );

        ::rtl::OUString getURL() const;
        ::rtl::OUString getFilterName() const;
};

#endif  // _FILEOPEN_DIALOG_HXX_

