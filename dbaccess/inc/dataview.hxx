/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dataview.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 07:49:00 $
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
#ifndef DBAUI_DATAVIEW_HXX
#define DBAUI_DATAVIEW_HXX

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX
#include <svtools/acceleratorexecute.hxx>
#endif
#include <memory>
#include "dbaccessdllapi.h"

class FixedLine;
class SvtMiscOptions;
namespace dbaui
{
    class IController;
    class DBACCESS_DLLPUBLIC ODataView :    public Window
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceFactory;  // the service factory to work with

    protected:
        IController*        m_pController;  // the controller in where we resides in
        FixedLine*          m_pSeparator;   // our separator above the toolbox (may be NULL)
        ::std::auto_ptr< ::svt::AcceleratorExecute> m_pAccel;

    public:
        ODataView(  Window* pParent,
                    IController* _pController,
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

        inline IController* getCommandController() const { return m_pController; }

        /** will be called when the controls need to be resized.
        */
        virtual void resizeControls(const Size& /*_rDiff*/) { Resize(); }

        void        enableSeparator( const sal_Bool _bEnable = sal_True );
        sal_Bool    isSeparatorEnabled() const { return NULL != m_pSeparator; }

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

