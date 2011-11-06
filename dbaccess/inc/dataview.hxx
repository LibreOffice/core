/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef DBAUI_DATAVIEW_HXX
#define DBAUI_DATAVIEW_HXX

#include "dbaccessdllapi.h"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <svtools/acceleratorexecute.hxx>
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

