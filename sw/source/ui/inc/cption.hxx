/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cption.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 18:23:20 $
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
#ifndef _CPTION_HXX
#define _CPTION_HXX

#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _ACTCTRL_HXX //autogen
#include <actctrl.hxx>
#endif


#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTENTENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XELEMENTACCESS_HPP_
#include <com/sun/star/container/XElementAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

class SwFldMgr;
class SwView;

#include <wrtsh.hxx>
#include "optload.hxx"
#include "swlbox.hxx"


class SwCaptionDialog : public SvxStandardDialog
{
    class CategoryBox : public ComboBox
    {
    public:
        CategoryBox( Window* pParent, const ResId& rResId )
            : ComboBox( pParent, rResId )
        {}

        virtual long    PreNotify( NotifyEvent& rNEvt );
    };

    FixedText    aTextText;
    Edit         aTextEdit;
    FixedLine    aSettingsFL;
    FixedText    aCategoryText;
    CategoryBox  aCategoryBox;
    FixedText    aFormatText;
    ListBox      aFormatBox;
    //#i61007# order of captions
    FixedText    aNumberingSeparatorFT;
    Edit         aNumberingSeparatorED;
    FixedText    aSepText;
    Edit         aSepEdit;
    FixedText    aPosText;
    ListBox      aPosBox;
    OKButton     aOKButton;
    CancelButton aCancelButton;
    HelpButton   aHelpButton;
    PushButton   aAutoCaptionButton;
    PushButton   aOptionButton;

    String       sNone;

    SwCaptionPreview    aPrevWin;

    SwView       &rView; //Suchen per aktive ::com::sun::star::sdbcx::View vermeiden.
    SwFldMgr     *pMgr;      //Ptr um das include zu sparen
    SelectionType eType;

    String       sCharacterStyle;
    String       sObjectName;
    bool         bCopyAttributes;
    bool        bOrderNumberingFirst; //#i61007# order of captions

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    xNameAccess;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >         xNamed;

    DECL_LINK( SelectHdl, ListBox * );
    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( OptionHdl, Button * );
    DECL_LINK( CaptionHdl, PushButton*);

    virtual void Apply();

    void    DrawSample();
    void    CheckButtonWidth();
    void    ApplyCaptionOrder(); //#i61007# order of captions

public:
     SwCaptionDialog( Window *pParent, SwView &rV );
    ~SwCaptionDialog();
};

#endif


