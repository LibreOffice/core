/*************************************************************************
 *
 *  $RCSfile: fontlb.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:00:56 $
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

#ifndef SVX_FONTLB_HXX
#define SVX_FONTLB_HXX

#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif

#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif


// Klasse fuer die Darstellung von schriftabhaengigen Strings
class SvLBoxFontString : public SvLBoxString
{
    private:

    Font            aPrivatFont;

public:
                    SvLBoxFontString( SvLBoxEntry*,USHORT nFlags,const XubString& rStr,
                                    const Font& aFont);
                    SvLBoxFontString();
                    ~SvLBoxFontString();

    void            InitViewData( SvLBox*,SvLBoxEntry*,SvViewDataItem* );
    void            Paint( const Point&, SvLBox& rDev, USHORT nFlags,SvLBoxEntry* );
    SvLBoxItem*     Create() const;
};

// Listbox mit schriftabhaengiger Darstellung

class SvxFontListBox : public SvTabListBox
{
    private:

        VirtualDevice   *pPrivatVDev;
        Font            aStandardFont;
        Font            aEntryFont;
        BOOL            bSettingFont;

    protected:

        virtual void    InitEntry(SvLBoxEntry*, const XubString&,const Image&,const Image&);

    public:

        SvxFontListBox(Window* pParent,const ResId& aResID);
        ~SvxFontListBox();

        void    InsertFontEntry( const String& rString, Font aActorFont);

        void        SelectEntryPos( USHORT nPos, BOOL bSelect=TRUE);
        ULONG       GetSelectEntryPos();
        XubString   GetSelectEntry();
        void        SetNoSelection();

};


#endif
