/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef DBAUI_DATAVIEW_HXX
#define DBAUI_DATAVIEW_HXX

#include "dbaccessdllapi.h"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <svtools/acceleratorexecute.hxx>
#include <sal/macros.h>
#include <vcl/fixed.hxx>

#include <memory>

class FixedLine;
class SvtMiscOptions;
namespace dbaui
{
    class IController;
    class DBACCESS_DLLPUBLIC ODataView :    public Window
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceFactory;  // the service factory to work with

    protected:
        IController&        m_rController;  // the controller in where we resides in
        FixedLine           m_aSeparator;
        ::std::auto_ptr< ::svt::AcceleratorExecute> m_pAccel;

    public:
        ODataView(  Window* pParent,
                    IController& _rController,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ,
                    WinBits nStyle = 0 );
        virtual ~ODataView();

        /// late construction
        virtual void Construct();
        // initialize will be called when after the controller finished his initialize method
        virtual void initialize(){}
        // window overridables
        virtual long PreNotify( NotifyEvent& rNEvt );
        virtual void StateChanged( StateChangedType nStateChange );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );

        inline IController& getCommandController() const { return m_rController; }

        /** will be called when the controls need to be resized.
        */
        virtual void resizeControls(const Size& /*_rDiff*/) { Resize(); }

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() { return m_xServiceFactory;}

        // the default implementation simply calls resizeAll( GetSizePixel() )
        virtual void Resize();

        void attachFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame);
    protected:
        // window overridables
        virtual void Paint( const Rectangle& _rRect );

        /// re-arrange all controls, including the toolbox, it's separator, and the "real view"
        virtual void resizeAll( const Rectangle& _rPlayground );

        // re-arrange the controls belonging to the document itself
        virtual void resizeDocumentView( Rectangle& _rPlayground );
    };
}
#endif // DBAUI_DATAVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
