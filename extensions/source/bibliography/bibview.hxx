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


#ifndef _BIB_VIEW_HXX
#define _BIB_VIEW_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include "formcontrolcontainer.hxx"
#include "bibshortcuthandler.hxx"

class   BibGeneralPage;
class   BibDataManager;

namespace com{ namespace sun{ namespace star{ namespace awt{ class XFocusListener;}}}}

//.........................................................................
namespace bib
{
//.........................................................................

    // -----------------------------------------------------------------------
    class BibView : public BibWindow, public FormControlContainer
    {
    private:
        BibDataManager*                                                             m_pDatMan;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable>        m_xDatMan;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener>    m_xGeneralPage;
        BibGeneralPage*                                                             m_pGeneralPage;

    private:
        DECL_STATIC_LINK(BibView, CallMappingHdl, BibView*);

    protected:
        // Window overridables
            virtual void    Resize();

        // FormControlContainer
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
                            getControlContainer();

        // XLoadListener equivalents
        virtual void        _loaded( const ::com::sun::star::lang::EventObject& _rEvent );
        virtual void        _reloaded( const ::com::sun::star::lang::EventObject& _rEvent );

    public:
                            BibView( Window* _pParent, BibDataManager* _pDatMan, WinBits nStyle = WB_3DLOOK );
                            ~BibView();

        void                UpdatePages();

        virtual void        GetFocus();

        virtual sal_Bool        HandleShortCutKey( const KeyEvent& rKeyEvent ); // returns true, if key was handled
    };

//.........................................................................
}   // namespace bib
//.........................................................................

#endif


