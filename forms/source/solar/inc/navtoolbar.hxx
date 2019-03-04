/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FORMS_SOURCE_SOLAR_INC_NAVTOOLBAR_HXX
#define INCLUDED_FORMS_SOURCE_SOLAR_INC_NAVTOOLBAR_HXX

#include <vcl/toolbox.hxx>
#include <vcl/field.hxx>

#include <memory>


namespace frm
{


    class IFeatureDispatcher;
    class ICommandImageProvider;
    class ICommandDescriptionProvider;
    class ImplNavToolBar;
    typedef std::shared_ptr< const ICommandImageProvider >  PCommandImageProvider;

    class NavigationToolBar final : public vcl::Window
    {
    public:
        enum ImageSize
        {
            eSmall,
            eLarge
        };

        enum FunctionGroup
        {
            ePosition,
            eNavigation,
            eRecordActions,
            eFilterSort
        };

    private:
        const IFeatureDispatcher*       m_pDispatcher;
        const std::shared_ptr< const ICommandImageProvider >
                                        m_pImageProvider;
        ImageSize                       m_eImageSize;
        VclPtr<ImplNavToolBar>          m_pToolbar;
        ::std::vector< VclPtr<vcl::Window> > m_aChildWins;
        const OUString                  m_sModuleId;

    public:
        NavigationToolBar(
            vcl::Window* _pParent,
            WinBits _nStyle,
            const PCommandImageProvider& _pImageProvider,
            const OUString & sModuleId
        );
        virtual ~NavigationToolBar( ) override;
        virtual void dispose() override;

        /** sets the dispatcher which is to be used for the features

            If the dispatcher is the same as the one which is currently set,
            then the states of the features are updated

            @param _pDispatcher
                the new (or old) dispatcher. The caller is responsible for
                ensuring the life time of the object does exceed the life time
                of the tool bar instance.
        */
        void    setDispatcher( const IFeatureDispatcher* _pDispatcher );

        /// enables or disables a given feature
        void    enableFeature( sal_Int16 _nFeatureId, bool _bEnabled );

        /// checks or unchecks a given feature
        void    checkFeature( sal_Int16 _nFeatureId, bool _bEnabled );

        /// sets the text of a given feature
        void    setFeatureText( sal_Int16 _nFeatureId, const OUString& _rText );

        /** retrieves the current image size
        */
        ImageSize    GetImageSize( ) const { return m_eImageSize; }

        /** sets the size of the images
        */
        void                SetImageSize( ImageSize _eSize );

        /** shows or hides a function group
        */
        void                ShowFunctionGroup( FunctionGroup _eGroup, bool _bShow );

        /** determines whether or not a given function group is currently visible
        */
        bool                IsFunctionGroupVisible( FunctionGroup _eGroup );

        // Window "overridables" (hiding the respective Window methods)
        void                SetControlBackground();
        void                SetControlBackground( const Color& rColor );
        void                SetTextLineColor( );
        void                SetTextLineColor( const Color& rColor );

    private:
        // Window overridables
        virtual void        Resize() override;
        virtual void        StateChanged( StateChangedType nType ) override;

        /// ctor implementation
        void implInit( );

        /// impl version of SetImageSize
        void    implSetImageSize( ImageSize _eSize );

        /// updates the images of our items
        void    implUpdateImages();

        /// enables or disables an item, plus possible dependent items
        void implEnableItem( sal_uInt16 _nItemId, bool _bEnabled );

        // iterating through item windows
        typedef void (NavigationToolBar::*ItemWindowHandler) (sal_uInt16, vcl::Window*) const;
        void    forEachItemWindow( ItemWindowHandler _handler );
        typedef void (*ItemWindowHandler2) (sal_uInt16, vcl::Window*, const void*);
        void    forEachItemWindow( ItemWindowHandler2 _handler, const void* _pParam );

        static void setItemBackground( sal_uInt16 /* _nItemId */, vcl::Window* _pItemWindow, const void* _pColor );
        static void setTextLineColor( sal_uInt16 /* _nItemId */, vcl::Window* _pItemWindow, const void* _pColor );
#if 0
        void setItemWindowZoom( sal_uInt16 /* _nItemId */, vcl::Window* _pItemWindow, const void* /* _pParam */ ) const;
#endif
        void setItemControlFont( sal_uInt16 /* _nItemId */, vcl::Window* _pItemWindow ) const;
        void setItemControlForeground( sal_uInt16 /* _nItemId */, vcl::Window* _pItemWindow ) const;
        void adjustItemWindowWidth( sal_uInt16 _nItemId, vcl::Window* _pItemWindow ) const;
        static void enableItemRTL( sal_uInt16 /*_nItemId*/, vcl::Window* _pItemWindow, const void* _pIsRTLEnabled );
    };

    class RecordPositionInput : public NumericField
    {
    private:
        const IFeatureDispatcher*   m_pDispatcher;

    public:
        RecordPositionInput( vcl::Window* _pParent );

        /** sets the dispatcher which is to be used for the features
        */
        void    setDispatcher( const IFeatureDispatcher* _pDispatcher );

    protected:
        // Window overridables
        virtual void LoseFocus() override;
        virtual void KeyInput( const KeyEvent& rKeyEvent ) override;

    private:
        void FirePosition( bool _bForce );
    };


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_SOLAR_INC_NAVTOOLBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
