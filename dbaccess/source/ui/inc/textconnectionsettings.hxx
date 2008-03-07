/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textconnectionsettings.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:24:54 $
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

#ifndef TEXTCONNECTIONSETTINGS_HXX
#define TEXTCONNECTIONSETTINGS_HXX

#include "propertystorage.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>

#include <memory>

class SfxItemSet;
//........................................................................
namespace dbaui
{
//........................................................................

    class OTextConnectionHelper;

    //====================================================================
    //= TextConnectionSettingsDialog
    //====================================================================
    class TextConnectionSettingsDialog : public ModalDialog
    {
    public:
        TextConnectionSettingsDialog( Window* _pParent, SfxItemSet& _rItems );
        ~TextConnectionSettingsDialog();

        /** initializes a set of PropertyStorage instaces, which are bound to
            the text-connection relevant items in our item sets
        */
        static void bindItemStorages( SfxItemSet& _rSet, PropertyValues& _rValues );

        virtual short   Execute();

    private:
        ::std::auto_ptr< OTextConnectionHelper >    m_pTextConnectionHelper;
        OKButton                                    m_aOK;
        CancelButton                                m_aCancel;
        SfxItemSet&                                 m_rItems;

    private:
        DECL_LINK( OnOK, PushButton* );
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // TEXTCONNECTIONSETTINGS_HXX
