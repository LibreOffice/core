/*************************************************************************
 *
 *  $RCSfile: futext.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:39 $
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

#ifndef _SD_FUTEXT_HXX
#define _SD_FUTEXT_HXX

#ifndef _EDITDATA_HXX
#include <svx/editdata.hxx>
#endif
#ifndef _SD_FUCONSTR_HXX
#include "fuconstr.hxx"
#endif

struct StyleRequestData;
class SdrTextObj;
class OutlinerParaObject;

/*************************************************************************
|*
|* Basisklasse fuer Textfunktionen
|*
\************************************************************************/
class FuText : public FuConstruct
{
 protected:
    SdrTextObj*         pTextObj;
    Link                aOldLink;
    BOOL                bFirstObjCreated;

    SfxRequest&         rRequest;

 public:
    TYPEINFO();

    FuText(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
           SdDrawDocument* pDoc, SfxRequest& rReq);

    virtual ~FuText();

    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);
    virtual BOOL Command(const CommandEvent& rCEvt);
    virtual BOOL RequestHelp(const HelpEvent& rHEvt);
    virtual void ReceiveRequest(SfxRequest& rReq);
    virtual void DoubleClick(const MouseEvent& rMEvt);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    void    SetInEditMode(const MouseEvent& rMEvt, BOOL bQuickDrag);
    BOOL    DeleteDefaultText();
    BOOL    RestoreDefaultText();
    void    ObjectChanged();
    SdrTextObj* GetTextObj() { return pTextObj; }
    void    SetSpellOptions( ULONG& rCntrl );

    // #71422: Start the functionality of this class in this method
    // and not in the ctor.
    // If you construct an object of this class and you put the
    // address of this object to pFuActual you got a problem,
    // because some methods inside DoExecute use the pFuActual-Pointer.
    // If the code inside DoExecute is executed inside the ctor,
    // the value of pFuActual is not right. And the value will not
    // be right until the ctor finished !!!
    void        DoExecute ();

    DECL_LINK(SpellError, void* );
};



#endif      // _SD_FUTEXT_HXX
