/*************************************************************************
 *
 *  $RCSfile: entrylisthelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 09:01:03 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef FORMS_ENTRYLISTHELPER_HXX
#define FORMS_ENTRYLISTHELPER_HXX

#ifndef _DRAFTS_COM_SUN_STAR_FORM_XLISTENTRYSINK_HDL_
#include <drafts/com/sun/star/form/XListEntrySink.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FORM_XLISTENTRYLISTENER_HDL_
#include <drafts/com/sun/star/form/XListEntryListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

    //=====================================================================
    //= OEntryListHelper
    //=====================================================================
    typedef ::cppu::ImplHelper2 <   ::drafts::com::sun::star::form::XListEntrySink
                                ,   ::drafts::com::sun::star::form::XListEntryListener
                                >   OEntryListHelper_BASE;

    class OEntryListHelper : public OEntryListHelper_BASE
    {
    private:
        ::osl::Mutex&   m_rMutex;

        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::form::XListEntrySource >
                        m_xListSource;      /// our external list source
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        m_aStringItems;     /// "overridden" StringItemList property value

    protected:
        OEntryListHelper( ::osl::Mutex& _rMutex );
        ~OEntryListHelper( );

        /// returns the current string item list
        inline const ::com::sun::star::uno::Sequence< ::rtl::OUString >&
                    getStringItemList() const { return m_aStringItems; }
        inline const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::form::XListEntrySource >&
                    getExternalListEntrySource() const { return m_xListSource; }

        /// determines whether we actually have an external list source
        inline bool hasExternalListSource( ) const { return m_xListSource.is(); }

        /** handling the XEventListener::disposing call for the case where
            our list source is being disposed
            @return
                <TRUE/> if and only if the disposed object was our list source, and so the
                event was handled
        */
        bool        handleDisposing( const ::com::sun::star::lang::EventObject& _rEvent );

        /** to be called by derived classes' instances when they're being disposed
        */
        void        disposing( );

        /** helper for implementing convertFastPropertyValue( StringItemList )

            <p>The signature of this method and the return type have the same semantics
            as convertFastPropertyValue.</p>
        */
        sal_Bool    convertNewListSourceProperty(
                        ::com::sun::star::uno::Any& _rConvertedValue,
                        ::com::sun::star::uno::Any& _rOldValue,
                        const ::com::sun::star::uno::Any& _rValue
                    )
                    SAL_THROW( ( ::com::sun::star::lang::IllegalArgumentException ) );

        /** helper for implementing setFastPropertyValueNoBroadcast

            <p>Will internally call stringItemListChanged after the new item list
            has been set.</p>

            @precond
                not to be called when we have an external list source
            @see hasExternalListSource
        */
        void        setNewStringItemList( const ::com::sun::star::uno::Any& _rValue );

        /** announces that the list of entries has changed.

            <p>Derived classes have to override this. Most probably, they'll set the new
            as model property.</p>

            @pure
            @see getStringItemList
        */
        virtual void    stringItemListChanged( ) = 0;

        /** called whenever a connection to a new external list source has been established
        */
        virtual void    connectedExternalListSource( );

        /** called whenever a connection to a new external list source has been revoked
        */
        virtual void    disconnectedExternalListSource( );

    private:
        // XListEntrySink
        virtual void SAL_CALL setListEntrySource( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::form::XListEntrySource >& _rxSource ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::form::XListEntrySource > SAL_CALL getListEntrySource(  ) throw (::com::sun::star::uno::RuntimeException);

        // XListEntryListener
        virtual void SAL_CALL entryChanged( const ::drafts::com::sun::star::form::ListEntryEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL entryRangeInserted( const ::drafts::com::sun::star::form::ListEntryEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL entryRangeRemoved( const ::drafts::com::sun::star::form::ListEntryEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL allEntriesChanged( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);
    private:
        /** disconnects from the active external list source, if present
            @see connectExternalListSource
        */
        void        disconnectExternalListSource( );

        /** connects to a new external list source
            @param _rxSource
                the new list source. Must not be <NULL/>
            @see disconnectExternalListSource
        */
        void        connectExternalListSource(
                        const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::form::XListEntrySource >& _rxSource
                    );

    private:
        OEntryListHelper();                                     // never implemented
        OEntryListHelper( const OEntryListHelper& );            // never implemented
        OEntryListHelper& operator=( const OEntryListHelper& ); // never implemented
    };

//.........................................................................
}   // namespace frm
//.........................................................................


#endif // FORMS_ENTRYLISTHELPER_HXX
