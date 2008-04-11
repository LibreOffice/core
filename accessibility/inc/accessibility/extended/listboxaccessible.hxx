/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: listboxaccessible.hxx,v $
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

#ifndef ACCESSIBILITY_EXT_LISTBOX_ACCESSIBLE
#define ACCESSIBILITY_EXT_LISTBOX_ACCESSIBLE

#include <com/sun/star/uno/RuntimeException.hpp>
#include <tools/link.hxx>

class SvTreeListBox;
class VclSimpleEvent;
class VclWindowEvent;

//........................................................................
namespace accessibility
{
//........................................................................

    //====================================================================
    //= ListBoxAccessibleBase
    //====================================================================
    /** helper class which couples it's life time to the life time of an
        SvTreeListBox
    */
    class ListBoxAccessibleBase
    {
    private:
        SvTreeListBox* m_pWindow;

    protected:
        inline SvTreeListBox*       getListBox() const
        {
            return  const_cast< ListBoxAccessibleBase* >( this )->m_pWindow;
        }

        inline  bool                    isAlive() const     { return NULL != m_pWindow; }

    public:
        ListBoxAccessibleBase( SvTreeListBox& _rWindow );

    protected:
        virtual ~ListBoxAccessibleBase( );

        // own overridables
        /// will be called for any VclWindowEvent events broadcasted by our VCL window
        virtual void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent );

        /** will be called when our window broadcasts the VCLEVENT_OBJECT_DYING event

            <p>Usually, you derive your class from both ListBoxAccessibleBase and XComponent,
            and call XComponent::dispose here.</p>
        */
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException ) = 0;

        /// to be called in the dispose method of your derived class
        void disposing();

    private:
        DECL_LINK( WindowEventListener, VclSimpleEvent* );

    private:
        ListBoxAccessibleBase( );                                           // never implemented
        ListBoxAccessibleBase( const ListBoxAccessibleBase& );              // never implemented
        ListBoxAccessibleBase& operator=( const ListBoxAccessibleBase& );   // never implemented
    };

//........................................................................
}   // namespace accessibility
//........................................................................

#endif // ACCESSIBILITY_EXT_LISTBOX_ACCESSIBLE
