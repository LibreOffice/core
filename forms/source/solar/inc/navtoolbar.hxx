/*************************************************************************
 *
 *  $RCSfile: navtoolbar.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-13 11:20:44 $
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

#ifndef FORMS_SOLAR_CONTROL_NAVTOOLBAR_HXX
#define FORMS_SOLAR_CONTROL_NAVTOOLBAR_HXX

#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

    class IFeatureDispatcher;

    class ImplNavToolBar;
    //=====================================================================
    //= NavigationToolBar
    //=====================================================================
    class NavigationToolBar : public Window
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
        const IFeatureDispatcher*   m_pDispatcher;
        ImageSize                   m_eImageSize;
        ImplNavToolBar*             m_pToolbar;
        ::std::vector< Window* >    m_aChildWins;

    public:
        NavigationToolBar( Window* _pParent, WinBits _nStyle );
        ~NavigationToolBar( );

        /** sets the dispatcher which is to be used for the features

            If the dispatcher is the same as the one which is currently set,
            then the states of the features are updated

            @param _pDispatcher
                the new (or old) dispatcher. The caller is reponsible for
                ensuring the life time of the object does exceed the life time
                of the tool bar instance.
        */
        void                        setDispatcher( const IFeatureDispatcher* _pDispatcher );

        /** returns the currently set dispatcher
        */
        const IFeatureDispatcher*   getDispatcher( );

        /** enables or disables a given feature
        */
        void                        enableFeature( sal_Int32 _nFeatureId, bool _bEnabled );

        /** checks or unchecks a given feature
        */
        void                        checkFeature( sal_Int32 _nFeatureId, bool _bEnabled );

        /** sets the text of a given feature
        */
        void                        setFeatureText( sal_Int32 _nFeatureId, const ::rtl::OUString& _rText );

        /** retrieves the current image size
        */
        inline ImageSize    GetImageSize( ) const { return m_eImageSize; }

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

    protected:
        // Window overridables
        virtual void        Resize();
        virtual void        StateChanged( StateChangedType nType );

        /// ctor implementation
        void implInit( );

        /// impl version of SetImageSize
        void    implSetImageSize( ImageSize _eSize, bool _bForce = false );

        /// enables or disables an item, plus possible dependent items
        void implEnableItem( USHORT _nItemId, bool _bEnabled );

        /** update the states of all features, using the callback
        */
        void updateFeatureStates( );

        // iterating through item windows
        typedef void (NavigationToolBar::*ItemWindowHandler) (USHORT, Window*, const void*) const;
        void    forEachItemWindow( ItemWindowHandler _handler, const void* _pParam );

        void setItemBackground( USHORT /* _nItemId */, Window* _pItemWindow, const void* _pColor ) const;
        void setTextLineColor( USHORT /* _nItemId */, Window* _pItemWindow, const void* _pColor ) const;
        void setItemWindowZoom( USHORT /* _nItemId */, Window* _pItemWindow, const void* /* _pParam */ ) const;
        void setItemControlFont( USHORT /* _nItemId */, Window* _pItemWindow, const void* /* _pParam */ ) const;
        void setItemControlForeground( USHORT /* _nItemId */, Window* _pItemWindow, const void* /* _pParam */ ) const;
        void adjustItemWindowWidth( USHORT _nItemId, Window* _pItemWindow, const void* /* _pParam */ ) const;
    };

    //=====================================================================
    //= RecordPositionInput
    //=====================================================================
    class RecordPositionInput : public NumericField
    {
    private:
        const IFeatureDispatcher*   m_pDispatcher;

    public:
        RecordPositionInput( Window* _pParent );
        ~RecordPositionInput();

        /** sets the dispatcher which is to be used for the features
        */
        void    setDispatcher( const IFeatureDispatcher* _pDispatcher );

    protected:
        // Window overridables
        virtual void LoseFocus();
        virtual void KeyInput( const KeyEvent& rKeyEvent );

    private:
        void FirePosition( sal_Bool _bForce );
    };

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // FORMS_SOLAR_CONTROL_NAVTOOLBAR_HXX
