/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: newdatatype.hxx,v $
 * $Revision: 1.4 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/edit.hxx>

#include <set>
#include <vector>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= NewDataTypeDialog
    //====================================================================
    class NewDataTypeDialog : public ModalDialog
    {
    private:
        FixedText       m_aLabel;
        Edit            m_aName;
        OKButton        m_aOK;
        CancelButton    m_aCancel;

        ::std::set< ::rtl::OUString >
                        m_aProhibitedNames;

    public:
        NewDataTypeDialog( Window* _pParent, const ::rtl::OUString& _rNameBase, const ::std::vector< ::rtl::OUString >& _rProhibitedNames );

        inline String   GetName() const { return m_aName.GetText(); }

    private:
        DECL_LINK( OnNameModified, void* );
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX

