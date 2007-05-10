/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sqlcommanddesign.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-10 10:50:01 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_SQLCOMMANDDESIGN_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_SQLCOMMANDDESIGN_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_XMULTICOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XOBJECTINSPECTORUI_HPP_
#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= SQLCommandDesigner
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::beans::XPropertyChangeListener
                                    >   SQLCommandDesigner_Base;
    /** encapsulates the code for calling and managing a query design frame, used
        for interactively designing the Command property of a ->RowSet
    */
    class SQLCommandDesigner : public SQLCommandDesigner_Base
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiComponentFactory >  m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >           m_xRowSet;
        ::dbtools::SharedConnection                                                         m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >            m_xDesigner;
        Link                                                                                m_aCloseLink;

    public:
        /** creates the instance, and immediately opens the SQL command design frame

        @param  _rxContext
            our component context. Must not be <NULL/>, and must provide a non-<NULL/> XMultiComponentFactory
        @param  _rxRowSet
            a ->RowSet whose Command property is to be designed. Must not be <NULL/>.
        @param  _rConnection
            the current connection of ->_rxRowSet. Must not be <NULL/>.
        @param _rCloseLink
            link to call when the component has been closed
        @throws ::com::sun::star::lang::NullPointerException
            if any of the arguments (except ->_rCloseLink) is <NULL/>, or if the component context
            does not provide a valid component factory.
        */
        SQLCommandDesigner(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxRowSet,
            const ::dbtools::SharedConnection& _rConnection,
            const Link& _rCloseLink
        );

        /** determines whether the SQL Command designer is currently active, i.e.
            if there currently exists a frame which allows the user entering the SQL command
        */
        inline bool isActive() const { return m_xDesigner.is(); }

        /** raises the designer window to top
            @precond
                the designer is active (->isActive)
            @precond
                the instance is not disposed
        */
        void    raise() const;

        /** suspends the designer
            @precond
                the designer is active (->isActive)
            @precond
                the instance is not disposed
        */
        bool    suspend() const;

        /** disposes the instance so that it becomes non-functional
        */
        void    dispose();

    protected:
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        ~SQLCommandDesigner();

        /** opens a new frame for interactively designing an SQL command
            @precond
                the designer is not currently active (see ->isActive)
            @precond
                ->m_xConnection is not <NULL/>
            @precond
                ->m_xRowSet is not <NULL/>
        */
        void impl_doOpenDesignerFrame_nothrow();

        /** impl-version of ->raise
        */
        void impl_raise_nothrow() const;

        /** determines whether we are already disposed
        */
        bool impl_isDisposed() const
        {
            return !m_xContext.is();
        }
        /** checks whether we are already disposed
            @throws ::com::sun::star::lang::DisposedException
                if we in fact are disposed
        */
        void impl_checkDisposed_throw() const;

        /** create an empty top-level frame, which does not belong to the desktop's frame list
            @precond
                ->m_xORB is not <NULL/>
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
            impl_createEmptyParentlessTask_nothrow() const;

        /** called whenever the component denoted by m_xDesigner has been closed
            <em>by an external instance</em>
        */
        void impl_designerClosed_nothrow();

        /** closes the component denoted by m_xDesigner
            @precond
                our designer component is actually active (->isActive)
            @precond
                we're not disposed already
        */
        void impl_closeDesigner_nothrow();

        /** suspends our designer component
            @precond
                the designer component is actually active (->isActive)
            @return
                <TRUE/> if the suspension was successful, <FALSE/> if it was vetoed
        */
        bool impl_trySuspendDesigner_nothrow() const;

    private:
        SQLCommandDesigner();                                       // never implemented
        SQLCommandDesigner( const SQLCommandDesigner& );            // never implemented
        SQLCommandDesigner& operator=( const SQLCommandDesigner& ); // never implemented
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_SQLCOMMANDDESIGN_HXX

