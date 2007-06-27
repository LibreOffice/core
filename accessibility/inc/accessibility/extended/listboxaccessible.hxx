/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listboxaccessible.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:21:44 $
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

#ifndef ACCESSIBILITY_EXT_LISTBOX_ACCESSIBLE
#define ACCESSIBILITY_EXT_LISTBOX_ACCESSIBLE

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

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
