/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewobjectcontactofunocontrol.hxx,v $
 * $Revision: 1.9 $
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

#ifndef SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX
#define SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX

#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/uno/Reference.hxx>
/** === end UNO includes === **/
#include <rtl/ref.hxx>
#include <svx/svxdllapi.h>

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
    class SVX_DLLPRIVATE ViewObjectContactOfUnoControl : public ViewObjectContactOfSdrObj
    {
    protected:
        ::rtl::Reference< ViewObjectContactOfUnoControl_Impl >    m_pImpl;

    public:
        /// determines whether an XControl already exists, and is currently visible
        bool    isControlVisible() const;

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

        /** positions the control for subsequent paint operations
        */
        void    positionControlForPaint( const DisplayInfo& _rDisplayInfo ) const;

        /** callback from impl class to react on changes of properties form the XControlModel
        */
        void propertyChange();

        /** React on changes of the object of this ViewContact
        */
        virtual void ActionChanged();

    protected:
        ViewObjectContactOfUnoControl( ObjectContact& _rObjectContact, ViewContactOfUnoControl& _rViewContact );
        ~ViewObjectContactOfUnoControl();

        // support for Primitive2D
        virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

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

    private:
        UnoControlPDFExportContact();                                               // never implemented
        UnoControlPDFExportContact( const UnoControlPDFExportContact& );            // never implemented
        UnoControlPDFExportContact& operator=( const UnoControlPDFExportContact& ); // never implemented
    };

//........................................................................
} } // namespace sdr::contact
//........................................................................

#endif // SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX

