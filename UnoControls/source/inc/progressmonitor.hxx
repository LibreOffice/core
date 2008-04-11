/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: progressmonitor.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _UNOCONTROLS_PROGRESSMONITOR_CTRL_HXX
#define _UNOCONTROLS_PROGRESSMONITOR_CTRL_HXX

//____________________________________________________________________________________________________________
//  includes of other projects
//____________________________________________________________________________________________________________

#include <com/sun/star/awt/XFileDialog.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XProgressMonitor.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/XScrollBar.hpp>
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XMessageBox.hpp>
#include <com/sun/star/awt/XTextEditField.hpp>
#include <com/sun/star/awt/Style.hpp>
#include <com/sun/star/awt/XTimeField.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XSpinField.hpp>
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#include <com/sun/star/awt/XNumericField.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XTextArea.hpp>
#include <com/sun/star/awt/XImageButton.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>
#include <com/sun/star/awt/XCurrencyField.hpp>
#include <com/sun/star/awt/XPatternField.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/awt/XTabController.hpp>
#include <com/sun/star/awt/XVclContainer.hpp>
#include <com/sun/star/awt/XDateField.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/MessageBoxCommand.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XProgressBar.hpp>

//____________________________________________________________________________________________________________
//  includes of my own project
//____________________________________________________________________________________________________________
#include "basecontainercontrol.hxx"

//____________________________________________________________________________________________________________
//  namespace
//____________________________________________________________________________________________________________

namespace unocontrols{

#define UNO3_ANY                                        ::com::sun::star::uno::Any
#define UNO3_OUSTRING                                   ::rtl::OUString
#define UNO3_RECTANGLE                                  ::com::sun::star::awt::Rectangle
#define UNO3_REFERENCE                                  ::com::sun::star::uno::Reference
#define UNO3_RUNTIMEEXCEPTION                           ::com::sun::star::uno::RuntimeException
#define UNO3_SEQUENCE                                   ::com::sun::star::uno::Sequence
#define UNO3_SIZE                                       ::com::sun::star::awt::Size
#define UNO3_TYPE                                       ::com::sun::star::uno::Type
#define UNO3_XACTIONLISTENER                            ::com::sun::star::awt::XActionListener
#define UNO3_XBUTTON                                    ::com::sun::star::awt::XButton
#define UNO3_XCONTROLMODEL                              ::com::sun::star::awt::XControlModel
#define UNO3_XFIXEDTEXT                                 ::com::sun::star::awt::XFixedText
#define UNO3_XGRAPHICS                                  ::com::sun::star::awt::XGraphics
#define UNO3_XLAYOUTCONSTRAINS                          ::com::sun::star::awt::XLayoutConstrains
#define UNO3_XMULTISERVICEFACTORY                       ::com::sun::star::lang::XMultiServiceFactory
#define UNO3_XPROGRESSBAR                               ::com::sun::star::awt::XProgressBar
#define UNO3_XPROGRESSMONITOR                           ::com::sun::star::awt::XProgressMonitor
#define UNO3_XTOOLKIT                                   ::com::sun::star::awt::XToolkit
#define UNO3_XWINDOWPEER                                ::com::sun::star::awt::XWindowPeer

//____________________________________________________________________________________________________________
//  defines
//____________________________________________________________________________________________________________

#define SERVICENAME_PROGRESSMONITOR                     "com.sun.star.awt.XProgressMonitor"
#define IMPLEMENTATIONNAME_PROGRESSMONITOR              "stardiv.UnoControls.ProgressMonitor"
#define FREEBORDER                                      10                                                      // border around and between the controls
#define WIDTH_RELATION                                  4                                                       // reserve 1/4 for button width and rest for progressbar width
#define HEIGHT_RELATION                                 5                                                       // reserve 1/5 for button and progressbar heigth and rest for text height
#define FIXEDTEXT_SERVICENAME                           "com.sun.star.awt.UnoControlFixedText"
#define BUTTON_SERVICENAME                              "com.sun.star.awt.UnoControlButton"
#define FIXEDTEXT_MODELNAME                             "com.sun.star.awt.UnoControlFixedTextModel"
#define BUTTON_MODELNAME                                "com.sun.star.awt.UnoControlButtonModel"
#define CONTROLNAME_TEXT                                "Text"                                                  // identifier the control in container
#define CONTROLNAME_BUTTON                              "Button"                                                //              -||-
#define CONTROLNAME_PROGRESSBAR                         "ProgressBar"                                           //              -||-
#define DEFAULT_BUTTONLABEL                             "Abbrechen"
#define DEFAULT_TOPIC                                   "\0"
#define DEFAULT_TEXT                                    "\0"
#define BACKGROUNDCOLOR                                 TRGB_COLORDATA( 0x00, 0xC0, 0xC0, 0xC0 )                // lighgray
#define LINECOLOR_BRIGHT                                TRGB_COLORDATA( 0x00, 0xFF, 0xFF, 0xFF )                // white
#define LINECOLOR_SHADOW                                TRGB_COLORDATA( 0x00, 0x00, 0x00, 0x00 )                // black
#define HEIGHT_PROGRESSBAR                              15
// Overwrite defines from basecontrol.hxx!!!
#undef  DEFAULT_WIDTH
#undef  DEFAULT_HEIGHT
#define DEFAULT_WIDTH                                   350
#define DEFAULT_HEIGHT                                  100

//____________________________________________________________________________________________________________
//  structs, types
//____________________________________________________________________________________________________________

/// Item of TextList
struct IMPL_TextlistItem
{
    UNO3_OUSTRING   sTopic  ;   /// Left site of textline in dialog
    UNO3_OUSTRING   sText   ;   /// Right site of textline in dialog
};

/// Define a list-class for struct IMPL_TextlistItem
class IMPL_Textlist ;
DECLARE_LIST( IMPL_Textlist, IMPL_TextlistItem* )

//____________________________________________________________________________________________________________
//  class declaration
//____________________________________________________________________________________________________________

class ProgressMonitor   : public UNO3_XLAYOUTCONSTRAINS
                        , public UNO3_XBUTTON
                        , public UNO3_XPROGRESSMONITOR
                        , public BaseContainerControl
{

//____________________________________________________________________________________________________________
//  public methods
//____________________________________________________________________________________________________________

public:

    //________________________________________________________________________________________________________
    //  construct/destruct
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    ProgressMonitor( const UNO3_REFERENCE< UNO3_XMULTISERVICEFACTORY >& xFactory );

    /**_______________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ~ProgressMonitor();

    //________________________________________________________________________________________________________
    //  XInterface
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual UNO3_ANY SAL_CALL queryInterface( const UNO3_TYPE& aType ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      increment refcount
        @descr      -

        @seealso    XInterface
        @seealso    release()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw();

    /**_______________________________________________________________________________________________________
        @short      decrement refcount
        @descr      -

        @seealso    XInterface
        @seealso    acquire()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw();

    //________________________________________________________________________________________________________
    //  XTypeProvider
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      get information about supported interfaces
        @descr      -

        @seealso    XTypeProvider

        @param      -

        @return     Sequence of types of all supported interfaces

        @onerror    A RuntimeException is thrown.
    */

    virtual UNO3_SEQUENCE< UNO3_TYPE > SAL_CALL getTypes() throw( UNO3_RUNTIMEEXCEPTION );

    //________________________________________________________________________________________________________
    //  XAggregation
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_ANY SAL_CALL queryAggregation( const UNO3_TYPE& aType ) throw( UNO3_RUNTIMEEXCEPTION );

    //________________________________________________________________________________________________________
    //  XProgressMonitor
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      add topic to dialog
        @descr      Add a topic with a text in right textlist (used for FixedText-member).<BR>
                    ( "beforeProgress" fix the right list ). The dialog metric is recalculated.

        @seealso    removeText(), updateText()

        @param      sTopic              Name of topic<BR>
                                         [sTopic != "" && sTopic != NULL]
        @param      sText               Value of topic<BR>
                                         [sText != "" && sText != NULL]
        @param      bbeforeProgress     Position of topic<BR>
                                         [True => before progressbar / False => below progressbar]

        @return     -

        @onerror    DEBUG   = Assertion<BR>
                    RELEASE = nothing
    */

    virtual void SAL_CALL addText(  const   UNO3_OUSTRING&  sTopic          ,
                                    const   UNO3_OUSTRING&  sText           ,
                                            sal_Bool        bbeforeProgress ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeText(   const   UNO3_OUSTRING&  sTopic          ,
                                                sal_Bool        bbeforeProgress ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL updateText(   const   UNO3_OUSTRING&  sTopic          ,
                                        const   UNO3_OUSTRING&  sText           ,
                                                sal_Bool        bbeforeProgress ) throw( UNO3_RUNTIMEEXCEPTION );

    //________________________________________________________________________________________________________
    //  XProgressBar
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setForegroundColor( sal_Int32 nColor ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setBackgroundColor( sal_Int32 nColor ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setValue( sal_Int32 nValue ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setRange( sal_Int32   nMin    ,
                                    sal_Int32   nMax    ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Int32 SAL_CALL getValue() throw( UNO3_RUNTIMEEXCEPTION );

    //________________________________________________________________________________________________________
    //  XButton
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addActionListener( const UNO3_REFERENCE< UNO3_XACTIONLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeActionListener( const UNO3_REFERENCE< UNO3_XACTIONLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setLabel( const UNO3_OUSTRING& sLabel ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setActionCommand( const UNO3_OUSTRING& sCommand ) throw( UNO3_RUNTIMEEXCEPTION );

    //________________________________________________________________________________________________________
    //  XLayoutConstrains
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_SIZE SAL_CALL getMinimumSize() throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_SIZE SAL_CALL getPreferredSize() throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_SIZE SAL_CALL calcAdjustedSize( const UNO3_SIZE& aNewSize ) throw( UNO3_RUNTIMEEXCEPTION );

    //________________________________________________________________________________________________________
    //  XControl
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL createPeer(   const   UNO3_REFERENCE< UNO3_XTOOLKIT >&    xToolkit    ,
                                        const   UNO3_REFERENCE< UNO3_XWINDOWPEER >& xParent     ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL setModel( const UNO3_REFERENCE< UNO3_XCONTROLMODEL >& xModel ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_REFERENCE< UNO3_XCONTROLMODEL > SAL_CALL getModel() throw( UNO3_RUNTIMEEXCEPTION );

    //________________________________________________________________________________________________________
    //  XComponent
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL dispose() throw( UNO3_RUNTIMEEXCEPTION );

    //________________________________________________________________________________________________________
    //  XWindow
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setPosSize(   sal_Int32   nX      ,
                                        sal_Int32   nY      ,
                                        sal_Int32   nWidth  ,
                                        sal_Int32   nHeight ,
                                        sal_Int16   nFlags  ) throw( UNO3_RUNTIMEEXCEPTION );

    //________________________________________________________________________________________________________
    //  BaseControl
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    static const UNO3_SEQUENCE< UNO3_OUSTRING > impl_getStaticSupportedServiceNames();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    static const UNO3_OUSTRING impl_getStaticImplementationName();

//____________________________________________________________________________________________________________
//  protected methods
//____________________________________________________________________________________________________________

protected:

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void impl_paint(        sal_Int32                           nX          ,
                                    sal_Int32                           nY          ,
                            const   UNO3_REFERENCE< UNO3_XGRAPHICS >&   xGraphics   );

//____________________________________________________________________________________________________________
// private methods
//____________________________________________________________________________________________________________

private:
    using BaseControl::impl_recalcLayout;
    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    void impl_recalcLayout();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    void impl_rebuildFixedText();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    void impl_cleanMemory();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    IMPL_TextlistItem* impl_searchTopic(    const   UNO3_OUSTRING&  sTopic          ,
                                                     sal_Bool       bbeforeProgress );

//____________________________________________________________________________________________________________
// debug methods
//____________________________________________________________________________________________________________

private:

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    #ifdef DBG_UTIL

    sal_Bool impl_debug_checkParameter( const UNO3_OUSTRING& sTopic, const UNO3_OUSTRING& sText, sal_Bool bbeforeProgress );    // addText, updateText
    sal_Bool impl_debug_checkParameter( const UNO3_OUSTRING& rTopic, sal_Bool bbeforeProgress );                                // removeText

    #endif
//____________________________________________________________________________________________________________
// private variables
//____________________________________________________________________________________________________________

private:

    IMPL_Textlist*                          m_pTextlist_Top         ;   // Elements before progress
    UNO3_REFERENCE< UNO3_XFIXEDTEXT >       m_xTopic_Top            ;   // (used, if parameter "beforeProgress"=sal_True in "addText, updateText, removeText")
    UNO3_REFERENCE< UNO3_XFIXEDTEXT >       m_xText_Top             ;

    IMPL_Textlist*                          m_pTextlist_Bottom      ;   // Elements below of progress
    UNO3_REFERENCE< UNO3_XFIXEDTEXT >       m_xTopic_Bottom         ;   // (used, if parameter "beforeProgress"=sal_False in "addText, updateText, removeText")
    UNO3_REFERENCE< UNO3_XFIXEDTEXT >       m_xText_Bottom          ;

    UNO3_REFERENCE< UNO3_XPROGRESSBAR >     m_xProgressBar          ;
    UNO3_REFERENCE< UNO3_XBUTTON >          m_xButton               ;
    UNO3_RECTANGLE                          m_a3DLine               ;

};  // class ProgressMonitor

}   // namespace unocontrols

#endif  // #ifndef _UNOCONTROLS_PROGRESSMONITOR_CTRL_HXX
