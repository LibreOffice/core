/*************************************************************************
 *
 *  $RCSfile: filtergrouping.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 11:28:00 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SFX2_FILTERGROUPING_HXX
#define SFX2_FILTERGROUPING_HXX

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#endif

#ifndef _SFX_FILEDLGIMPL_HXX
#include "filedlgimpl.hxx"
#endif

class SfxFilterMatcherIter;

//........................................................................
namespace sfx2
{
//........................................................................

    //--------------------------------------------------------------------
    /** adds the given filters to the filter manager.
        <p>To be used when saving generic files.</p>
    */
    void appendFiltersForSave(
        SfxFilterMatcherIter& _rFilterMatcher,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilterManager >& _rFilterManager,
        ::rtl::OUString& /* [out] */ _rFirstNonEmpty,
        FileDialogHelper_Impl& _rFileDlgImpl
    );

    void appendExportFilters(
        SfxFilterMatcherIter& _rFilterMatcher,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilterManager >& _rFilterManager,
        ::rtl::OUString& /* [out] */ _rFirstNonEmpty,
        FileDialogHelper_Impl& _rFileDlgImpl
        );

    //--------------------------------------------------------------------
    /** adds the given filters to the filter manager.
        <p>To be used when opening generic files.</p>
    */
    void appendFiltersForOpen(
        SfxFilterMatcherIter& _rFilterMatcher,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilterManager >& _rFilterManager,
        ::rtl::OUString& /* [out] */ _rFirstNonEmpty,
        FileDialogHelper_Impl& _rFileDlgImpl
    );

    //--------------------------------------------------------------------
    /** adds the given extension to the display text.
        <p>To be used when opening or save generic files.</p>
    */
    ::rtl::OUString addExtension(
        const ::rtl::OUString& _rDisplayText,
        const ::rtl::OUString& _rExtension,
        sal_Bool _bForOpen,
        FileDialogHelper_Impl& _rFileDlgImpl
    );

//........................................................................
}   // namespace sfx2
//........................................................................

#endif // SFX2_FILTERGROUPING_HXX


