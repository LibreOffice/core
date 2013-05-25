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


#ifndef _SFXMNUITEM_HXX
#define _SFXMNUITEM_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

class SfxVirtualMenu;
class SfxBindings;
class SfxModule;
class Menu;
class PopupMenu;
class SfxUnoMenuControl;
class SfxUnoControllerItem;
struct SfxMenuCtrlFactory;
#include <tools/string.hxx>
#include <sfx2/ctrlitem.hxx>

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/URL.hpp>

class SFX2_DLLPUBLIC SfxMenuControl: public SfxControllerItem
{
//friend SvStream& operator<<( SvStream& rStream, const SfxMenuControl& rItem );
//friend SvStream& operator>>( SvStream& rStream, SfxMenuControl& rItem );

    String                  aTitle;
    String                  aHelpText;
    SfxVirtualMenu*         pOwnMenu;
    SfxVirtualMenu*         pSubMenu;
    sal_Bool                    b_ShowStrings;
    sal_Bool                    b_UnusedDummy;

public:
                            SfxMenuControl();
                            SfxMenuControl( sal_Bool bShowStrings );
                            SfxMenuControl( sal_uInt16, SfxBindings&);

    static SfxMenuControl*  CreateImpl( sal_uInt16 nId, Menu &rMenu, SfxBindings &rBindings );
    static void             RegisterControl( sal_uInt16 nSlotId = 0, SfxModule *pMod=NULL );

                            ~SfxMenuControl();

        using SfxControllerItem::Bind;
    void                    Bind( SfxVirtualMenu* pOwnMenu, sal_uInt16 nId,
                                  const String& rTitle, const String &rHelpText,
                                  SfxBindings & );
    void                    Bind( SfxVirtualMenu* pOwnMenu, sal_uInt16 nId,
                                  SfxVirtualMenu& rSubMenu,
                                  const String& rTitle, const String &rHelpText,
                                  SfxBindings & );

//  SvStream &              Load(SvStream &, SfxBindings*);
//  SvStream &              Store(SvStream &);

    String                  GetTitle() const;
    SfxVirtualMenu*         GetPopupMenu() const;
    virtual PopupMenu*      GetPopup() const;
    void                    SetOwnMenu( SfxVirtualMenu* pMenu );
    void                    RemovePopup();

    const String&           GetHelpText() const { return aHelpText; }
    void                    SetHelpText(const String &rStr) { aHelpText  = rStr; }

    virtual void            StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                          const SfxPoolItem* pState );

    static SfxMenuControl*    CreateControl( sal_uInt16 nId, Menu &, SfxBindings & );
    static SfxUnoMenuControl* CreateControl( const String&, sal_uInt16, Menu&, SfxBindings&, SfxVirtualMenu* );
    static SfxUnoMenuControl* CreateControl( const String&, sal_uInt16, Menu&, const String& sItemText, const String& sHelpText, SfxBindings&, SfxVirtualMenu* );
    static sal_Bool             IsSpecialControl( sal_uInt16 nId, SfxModule* );
    static void             RegisterMenuControl(SfxModule*, SfxMenuCtrlFactory*);

};

class SfxUnoMenuControl : public SfxMenuControl
{
    SfxUnoControllerItem*   pUnoCtrl;
public:
                            SfxUnoMenuControl( const String&, sal_uInt16 nId, Menu&,
                                                SfxBindings&, SfxVirtualMenu* );
                            SfxUnoMenuControl( const String&, sal_uInt16 nId, Menu&,
                                               const String&, const String&,
                                                SfxBindings&, SfxVirtualMenu* );
                            ~SfxUnoMenuControl();
    void                    Select();
};

//--------------------------------------------------------------------

typedef SfxMenuControl* (*SfxMenuControlCtor)( sal_uInt16 nId, Menu &, SfxBindings & );

struct SfxMenuCtrlFactory
{
    SfxMenuControlCtor  pCtor;
    TypeId              nTypeId;
    sal_uInt16              nSlotId;

    SfxMenuCtrlFactory( SfxMenuControlCtor pTheCtor,
            TypeId nTheTypeId, sal_uInt16 nTheSlotId ):
        pCtor(pTheCtor),
        nTypeId(nTheTypeId),
        nSlotId(nTheSlotId)
    {}
};

//

inline String SfxMenuControl::GetTitle() const
{
    return aTitle;
}
//--------------------------------------------------------------------

//

inline SfxVirtualMenu* SfxMenuControl::GetPopupMenu() const
{
    return pSubMenu;
}
//--------------------------------------------------------------------

#define SFX_DECL_MENU_CONTROL() \
        static SfxMenuControl* CreateImpl( sal_uInt16 nId, Menu &rMenu, SfxBindings &rBindings ); \
        static void RegisterControl(sal_uInt16 nSlotId = 0, SfxModule *pMod=NULL)

#define SFX_IMPL_MENU_CONTROL(Class, nItemClass) \
        SfxMenuControl* __EXPORT Class::CreateImpl( sal_uInt16 nId, Menu &rMenu, SfxBindings &rBindings ) \
               { return new Class(nId, rMenu, rBindings); } \
        void Class::RegisterControl(sal_uInt16 nSlotId, SfxModule *pMod) \
               { SfxMenuControl::RegisterMenuControl( pMod, new SfxMenuCtrlFactory( \
                    Class::CreateImpl, TYPE(nItemClass), nSlotId ) ); }

//#if 0 // _SOLAR__PRIVATE

class SfxAppMenuControl_Impl : public SfxMenuControl
{
    PopupMenu*  pMenu;
    sal_uIntPtr       m_nSymbolsStyle;
    sal_Bool        m_bWasHiContrastMode;
    sal_Bool        m_bShowMenuImages;

protected:
    DECL_LINK( Activate, Menu * ); // Needed to support high contrast images

public:
    SFX_DECL_MENU_CONTROL();
    SfxAppMenuControl_Impl( sal_uInt16 nPos, Menu& rMenu, SfxBindings& rBindings );
    ~SfxAppMenuControl_Impl();

    struct ExecuteInfo
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
        ::com::sun::star::util::URL                                                aTargetURL;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >  aArgs;
    };

    DECL_STATIC_LINK( SfxAppMenuControl_Impl, ExecuteHdl_Impl, ExecuteInfo* );
};

//#endif

#endif
