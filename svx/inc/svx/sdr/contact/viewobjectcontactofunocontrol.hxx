/*************************************************************************
 *
 *  $RCSfile: viewobjectcontactofunocontrol.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:06:21 $
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

#ifndef SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX
#define SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
/** === end UNO includes === **/

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef INCLUDED_SVXDLLAPI_H
#include "svxdllapi.h"
#endif

class OutputDevice;
class Window;
class SdrUnoObj;
namespace com { namespace sun { namespace star {
    namespace awt {
        class XControl;
        class XControlContainer;
    }
} } }

//........................................................................
namespace sdr { namespace contact {
//........................................................................

    class ViewContactOfUnoControl;
    class ObjectContactOfPageView;
    //====================================================================
    //= ViewObjectContactOfUnoControl
    //====================================================================
    class ViewObjectContactOfUnoControl_Impl;
    class SVX_DLLPRIVATE ViewObjectContactOfUnoControl : public ViewObjectContact
    {
    protected:
        ::rtl::Reference< ViewObjectContactOfUnoControl_Impl >    m_pImpl;

    public:
        /// returns the ->XControl instance belonging to the instance, if has already been created
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >
                getExistentControl() const;

        /// returns the ->XControl instance belonging to the instance, creates it if necessary
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >
                getControl();

        /** retrieves a temporary XControl instance, whose parent is the given device
            @seealso SdrUnoObj::GetTemporaryControlForWindow
        */
        static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >
            getTemporaryControlForWindow(
                const Window& _rWindow,
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& _inout_ControlContainer,
                const SdrUnoObj& _rUnoObject
            );

        /// ensures that the control belonging to this instances has a given visibility
        void    ensureControlVisibility( bool _bVisible ) const;

        /** sets the design/alive mode of the control
        */
        void    setControlDesignMode( bool _bDesignMode ) const;

        /** determines whether the instance belongs to a given OutputDevice
            @precond
                The instance knows the device it belongs to, or can determine it.
                If this is not the case, you will notice an assertion, and the method will
                return false.
        */
        bool    belongsToDevice( const OutputDevice* _pDevice ) const;

    protected:
        ViewObjectContactOfUnoControl( ObjectContact& _rObjectContact, ViewContactOfUnoControl& _rViewContact );
        ~ViewObjectContactOfUnoControl();

        // ViewObjectContact overridables
        virtual void PaintObject( DisplayInfo& _rDisplayInfo );

        // own overridables
        /** paint the control

            All necessary preparations have been done, the method just needs to draw the control
            onto the device.

            The control to be painted can be obtained at our impl class, and is guaranteed to not be <NULL/>
            when this method is called.

            @param _rDisplayInfo
                the display info from the ->PaintObject call

            @param _pObject
                the SdrObject for the control to paint. Guaranteed to not be <NULL/>.
        */
        virtual void doPaintObject(
            const DisplayInfo& _rDisplayInfo,
            const SdrUnoObj* _pUnoObject
        ) const = 0;

    private:
        ViewObjectContactOfUnoControl();                                                  // never implemented
        ViewObjectContactOfUnoControl( const ViewObjectContactOfUnoControl& );              // never implemented
        ViewObjectContactOfUnoControl& operator=( const ViewObjectContactOfUnoControl& );   // never implemented
    };

    //====================================================================
    //= UnoControlDefaultContact
    //====================================================================
    class SVX_DLLPRIVATE UnoControlDefaultContact : public ViewObjectContactOfUnoControl
    {
    public:
        UnoControlDefaultContact( ObjectContact& _rObjectContact, ViewContactOfUnoControl& _rViewContact );
        ~UnoControlDefaultContact();

        // ViewObjectContactOfUnoControl overridables
        virtual void doPaintObject(
            const DisplayInfo& _rDisplayInfo,
            const SdrUnoObj* _pUnoObject
        ) const;

    private:
        UnoControlDefaultContact();                                             // never implemented
        UnoControlDefaultContact( const UnoControlDefaultContact& );            // never implemented
        UnoControlDefaultContact& operator=( const UnoControlDefaultContact& ); // never implemented
    };

    //====================================================================
    //= UnoControlWindowContact
    //====================================================================
    class SVX_DLLPRIVATE UnoControlWindowContact : public ViewObjectContactOfUnoControl
    {
    public:
        UnoControlWindowContact( ObjectContactOfPageView& _rObjectContact, ViewContactOfUnoControl& _rViewContact );
        ~UnoControlWindowContact();

        // ViewObjectContactOfUnoControl overridables
        virtual void doPaintObject(
            const DisplayInfo& _rDisplayInfo,
            const SdrUnoObj* _pUnoObject
        ) const;

    private:
        UnoControlWindowContact();                                              // never implemented
        UnoControlWindowContact( const UnoControlWindowContact& );              // never implemented
        UnoControlWindowContact& operator=( const UnoControlWindowContact& );   // never implemented
    };

    //====================================================================
    //= UnoControlPrintOrPreviewContact
    //====================================================================
    class SVX_DLLPRIVATE UnoControlPrintOrPreviewContact : public ViewObjectContactOfUnoControl
    {
    public:
        UnoControlPrintOrPreviewContact( ObjectContactOfPageView& _rObjectContact, ViewContactOfUnoControl& _rViewContact );
        ~UnoControlPrintOrPreviewContact();

        // ViewObjectContactOfUnoControl overridables
        virtual void doPaintObject(
            const DisplayInfo& _rDisplayInfo,
            const SdrUnoObj* _pUnoObject
        ) const;

    private:
        UnoControlPrintOrPreviewContact();                                                 // never implemented
        UnoControlPrintOrPreviewContact( const UnoControlPrintOrPreviewContact& );            // never implemented
        UnoControlPrintOrPreviewContact& operator=( const UnoControlPrintOrPreviewContact& ); // never implemented
    };

    //====================================================================
    //= UnoControlPDFExportContact
    //====================================================================
    class SVX_DLLPRIVATE UnoControlPDFExportContact : public ViewObjectContactOfUnoControl
    {
    public:
        UnoControlPDFExportContact( ObjectContactOfPageView& _rObjectContact, ViewContactOfUnoControl& _rViewContact );
        ~UnoControlPDFExportContact();

        // ViewObjectContactOfUnoControl overridables
        virtual void doPaintObject(
            const DisplayInfo& _rDisplayInfo,
            const SdrUnoObj* _pUnoObject
        ) const;

    private:
        UnoControlPDFExportContact();                                               // never implemented
        UnoControlPDFExportContact( const UnoControlPDFExportContact& );            // never implemented
        UnoControlPDFExportContact& operator=( const UnoControlPDFExportContact& ); // never implemented
    };

//........................................................................
} } // namespace sdr::contact
//........................................................................

#endif // SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX

