/*************************************************************************
 *
 *  $RCSfile: FieldDescControl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-22 07:47:44 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_FIELDDESCRIPTIONCONTROL_HXX
#define DBAUI_FIELDDESCRIPTIONCONTROL_HXX

#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif

//------------------------------------------------------------------
// die Spalten einer Feld-Beschreibung einer Tabelle
#define FIELD_NAME      1
#define FIELD_TYPE      2
#define FIELD_DESCR     3

#define FIELD_FIRST_VIRTUAL_COLUMN  4

#define FIELD_PROPERTY_REQUIRED     4
#define FIELD_PROPERTY_NUMTYPE      5
#define FIELD_PROPERTY_AUTOINC      6
#define FIELD_PROPERTY_DEFAULT      7
#define FIELD_PROPERTY_TEXTLEN      8
#define FIELD_PROPERTY_LENGTH       9
#define FIELD_PROPERTY_SCALE        10
#define FIELD_PROPERTY_BOOL_DEFAULT 11
#define FIELD_PROPERTY_FORMAT       12
#define FIELD_PRPOERTY_COLUMNNAME   13
#define FIELD_PRPOERTY_TYPE         14

class FixedText;
class PushButton;
class SvxNumberFormatShell;
class ScrollBar;
class Button;
class ListBox;
class Control;
class SvNumberFormatter;

namespace dbaui
{

    class OTableDesignHelpBar;
    class OPropListBoxCtrl;
    class OPropEditCtrl;
    class OPropNumericEditCtrl;
    class OFieldDescription;
    class OTypeInfo;
    class OPropColumnEditCtrl;
    //==================================================================
    class OFieldDescControl : public TabPage
    {
    private:
        OTableDesignHelpBar*    pHelp;
        Window*                 pLastFocusWindow;
        Window*                 m_pActFocusWindow;

        FixedText*              pDefaultText;
        FixedText*              pRequiredText;
        FixedText*              pAutoIncrementText;
        FixedText*              pTextLenText;
        FixedText*              pNumTypeText;
        FixedText*              pLengthText;
        FixedText*              pScaleText;
        FixedText*              pFormatText;
        FixedText*              pBoolDefaultText;
        FixedText*              m_pColumnNameText;
        FixedText*              m_pTypeText;

        OPropListBoxCtrl*       pRequired;
        OPropListBoxCtrl*       pNumType;
        OPropListBoxCtrl*       pAutoIncrement;
        OPropEditCtrl*          pDefault;
        OPropNumericEditCtrl*   pTextLen;
        OPropNumericEditCtrl*   pLength;
        OPropNumericEditCtrl*   pScale;
        OPropEditCtrl*          pFormatSample;
        OPropListBoxCtrl*       pBoolDefault;
        OPropColumnEditCtrl*    m_pColumnName;
        OPropListBoxCtrl*       m_pType;

        PushButton*             pFormat;

        ScrollBar*              m_pVertScroll;
        ScrollBar*              m_pHorzScroll;

        const OTypeInfo*        m_pPreviousType;
        USHORT                  nCurChildId;
        short                   nPos;
        short                   nOldDefaultPos;
        XubString               aYes;
        XubString               aNo;

        long                    m_nOldVThumb;
        long                    m_nOldHThumb;

        ULONG                   nDelayedGrabFocusEvent;

        DECL_LINK( OnScroll, ScrollBar*);

        DECL_LINK( FormatClickHdl, Button * );
        DECL_LINK( ChangeHdl, ListBox * );

        DECL_LINK( DelayedGrabFocus, Control** );
            // von ActivatePropertyField benutzt
        DECL_LINK( OnControlFocusLost, Control* );
        DECL_LINK( OnControlFocusGot, Control* );

        void                UpdateFormatSample(OFieldDescription* pFieldDescr);
        void                ArrangeAggregates();

        void                SetPosSize( Control** ppControl, long nRow, USHORT nCol );

        void                ScrollAggregate(Control* pText, Control* pInput, Control* pButton, long nDeltaX, long nDeltaY);
        void                ScrollAllAggregates();

    protected:
        OFieldDescription*      pActFieldDescr; // falls geloescht werden soll

        USHORT              CountActiveAggregates() const;

        virtual void        ActivateAggregate( EControlType eType );
        virtual void        DeactivateAggregate( EControlType eType );
        virtual BOOL        IsReadOnly() { return FALSE; };

        // Sind von den abgeleiteten Klassen zu impl.
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > GetFormatter() = 0;

        virtual ::com::sun::star::lang::Locale  GetLocale() const = 0;

        virtual void        CellModified(long nRow, USHORT nColId ) = 0;
        virtual void        SetModified(BOOL bModified)             = 0;

        virtual const OTypeInfo*    getTypeInfo(sal_Int32 _nPos)        = 0;
        virtual const OTypeInfoMap* getTypeInfo() const  = 0;

        String  BoolStringPersistent(const String& rUIString) const;
        String  BoolStringUI(const String& rPersistentString) const;

    public:
        OFieldDescControl( Window* pParent, OTableDesignHelpBar* pHelpBar);
        OFieldDescControl( Window* pParent, const ResId& rResId, OTableDesignHelpBar* pHelpBar);
        virtual ~OFieldDescControl();

        void                DisplayData(OFieldDescription* pFieldDescr );
        //  void                DisplayData(const OColumn* pColumn);

        void                SaveData( OFieldDescription* pFieldDescr );
        //  void                SaveData( OColumn* pColumn);

        void                SetControlText( USHORT nControlId, const String& rText );
        String              GetControlText( USHORT nControlId );
        void                SetReadOnly( BOOL bReadOnly );

        void                ActivatePropertyField(USHORT nVirtualField);
            // Parameter ist einer der FIELD_PROPERTY_xxx-Werte, das entsprechende Control wird aktiviert, wenn vorhanden

        // Resize aufegrufen
        void                CheckScrollBars();
        sal_Bool            isCutAllowed();
        void                cut();
        void                copy();
        void                paste();

        virtual void        Init();
        virtual void        GetFocus();
        virtual void        LoseFocus();
        virtual void        Resize();

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData() = 0;
    protected:
        void    implFocusLost(Window* _pWhich);
    };
}
#endif // DBAUI_FIELDDESCRIPTIONCONTROL_HXX


