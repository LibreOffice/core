/*************************************************************************
 *
 *  $RCSfile: CustomAnimationPane.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:55:26 $
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

#ifndef _SD_CUSTOMANIMATIONPANE_HXX
#define _SD_CUSTOMANIMATIONPANE_HXX

#ifndef _COM_SUN_STAR_DRAWING_XDRAWVIEW_HPP_
#include <com/sun/star/drawing/XDrawView.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

#ifndef _SD_CUSTOMANIMATIONPRESET_HXX
#include "CustomAnimationPreset.hxx"
#endif
#ifndef _SD_CUSTOMANIMATIONLIST_HXX
#include "CustomAnimationList.hxx"
#endif

class PushButton;
class FixedLine;
class FixedText;
class ListBox;
class ComboBox;
class CheckBox;
class SdDrawDocument;

namespace com { namespace sun { namespace star { namespace animations {
    class XAnimationNode;
} } } }

namespace sd { namespace tools {
class EventMultiplexerEvent;
} }

namespace sd {

class PropertyControl;
class STLPropertySet;
class ViewShellBase;

// --------------------------------------------------------------------

class CustomAnimationPane : public Control, public ICustomAnimationListController
{
public:
    CustomAnimationPane( ::Window* pParent, ViewShellBase& rBase, const Size& rMinSize );
    virtual ~CustomAnimationPane();

    // callbacks
    void onSelectionChanged();
    void onChangeCurrentPage();
    void onChange( bool bCreate );
    void onRemove();
    void onChangeStart();
    void onChangeStart( sal_Int16 nNodeType );
    void onChangeProperty();
    void onChangeSpeed();

    // methods
    void preview( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xAnimationNode );

    // Control
   virtual void Resize();

    // ICustomAnimationListController
    virtual void onSelect();
    virtual void onDoubleClick();
    virtual void onContextMenu( USHORT nSelectedPopupEntry );

    void addUndo();

private:
    void addListener();
    void removeListener();
    void updateLayout();
    void updateControls();

    void showOptions( USHORT nPage = 0 );
    void moveSelection( bool bUp );
    void onPreview( bool bForcePreview );

    STLPropertySet* createSelectionSet();
    void changeSelection( STLPropertySet* pResultSet, STLPropertySet* pOldSet );

    ::com::sun::star::uno::Any getProperty1Value( sal_Int32 nType, CustomAnimationEffectPtr pEffect );
    bool setProperty1Value( sal_Int32 nType, CustomAnimationEffectPtr pEffect, const ::com::sun::star::uno::Any& rValue );

    DECL_LINK( implControlHdl, Control* );
    DECL_LINK( implPropertyHdl, Control* );
    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent*);

private:
    ViewShellBase& mrBase;

    const CustomAnimationPresets& mrPresets;

    FixedLine*  mpFLModify;
    PushButton* mpPBAddEffect;
    PushButton* mpPBChangeEffect;
    PushButton* mpPBRemoveEffect;
    FixedLine*  mpFLEffect;
    FixedText*  mpFTStart;
    ListBox*    mpLBStart;
    FixedText*  mpFTProperty;
    PropertyControl*    mpLBProperty;
    PushButton* mpPBPropertyMore;
    FixedText*  mpFTSpeed;
    ComboBox*   mpCBSpeed;
    CustomAnimationList*    mpCustomAnimationList;
    FixedText*  mpFTChangeOrder;
    PushButton* mpPBMoveUp;
    PushButton* mpPBMoveDown;
    FixedLine*  mpFLSeperator1;
    PushButton* mpPBPlay;
    PushButton* mpPBSlideShow;
    FixedLine*  mpFLSeperator2;
    CheckBox*   mpCBAutoPreview;

    String      maStrModify;
    String      maStrProperty;

    sal_Int32   mnPropertyType;

    Size        maMinSize;

    EffectSequence maListSelection;
    ::com::sun::star::uno::Any maViewSelection;

    MainSequencePtr mpMainSequence;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > mxCurrentPage;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawView > mxView;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxModel;
};

};

#endif // _SD_CUSTOMANIMATIONPANE_HXX
