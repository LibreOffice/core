/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: newdatatype.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:19:54 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif

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

