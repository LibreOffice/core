/*************************************************************************
 *
 *  $RCSfile: dcontact.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:25 $
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
#ifndef _DCONTACT_HXX
#define _DCONTACT_HXX

#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#include "calbck.hxx"

class SfxPoolItem;
class SwFrmFmt;
class SwFlyFrmFmt;
class SwDrawFrmFmt;
class SwFlyFrm;
class SwFrm;
class SwPageFrm;
class SwVirtFlyDrawObj;
class SwFmtAnchor;
class SwFlyDrawObj;
class SwRect;

//Der Umgekehrte Weg: Sucht das Format zum angegebenen Objekt.
//Wenn das Object ein SwVirtFlyDrawObj ist so wird das Format von
//selbigem besorgt.
//Anderfalls ist es eben ein einfaches Zeichenobjekt. Diese hat einen
//UserCall und der ist Client vom gesuchten Format.
//Implementierung in dcontact.cxx
SwFrmFmt *FindFrmFmt( SdrObject *pObj );
inline const SwFrmFmt *FindFrmFmt( const SdrObject *pObj )
{   return ::FindFrmFmt( (SdrObject*)pObj ); }

//Bei Aenderungen das Objekt aus dem ContourCache entfernen.
//Implementierung in TxtFly.Cxx
void ClrContourCache( const SdrObject *pObj );

// liefert BoundRect inklusive Abstand
SwRect GetBoundRect( const SdrObject* pObj );

//Liefert den UserCall ggf. vom Gruppenobjekt
SdrObjUserCall* GetUserCall( const SdrObject* );

// liefert TRUE falls das SrdObject ein Marquee-Object (Lauftext) ist
FASTBOOL IsMarqueeTextObj( const SdrObject& rObj );

//Basisklasse fuer die folgenden KontaktObjekte (Rahmen+Zeichenobjekte)
class SwContact : public SdrObjUserCall, public SwClient
{
    SdrObject *pMasterObj;
public:
    TYPEINFO();

    //Fuer den Reader, es wir nur die Verbindung hergestellt.
    SwContact( SwFrmFmt *pToRegisterIn, SdrObject *pObj );
    SwContact( SwModify *pToRegisterIn ) : SwClient( pToRegisterIn ){}

    const SdrObject *GetMaster() const  { return pMasterObj; }
          SdrObject *GetMaster()        { return pMasterObj; }
    void SetMaster( SdrObject* pNew ){ pMasterObj = pNew; }

    virtual ~SwContact();

          SwFrmFmt  *GetFmt(){ return (SwFrmFmt*)GetRegisteredIn(); }
    const SwFrmFmt  *GetFmt() const
        { return (const SwFrmFmt*)GetRegisteredIn(); }
};

//KontactObjekt fuer die Verbindung zwischen Rahmen bzw. deren Formaten
//im StarWriter (SwClient) und den Zeichenobjekten des Drawing (SdrObjUserCall)

class SwFlyDrawContact : public SwContact
{
public:
    TYPEINFO();

    //Legt das DrawObjekt an und meldet es beim Model an.
    SwFlyDrawContact( SwFlyFrmFmt *pToRegisterIn, SdrModel *pMod );
    SwFlyDrawContact( SwFrmFmt *pToRegisterIn, SdrObject *pObj ):
        SwContact( pToRegisterIn, pObj ){}

    SwVirtFlyDrawObj *CreateNewRef( SwFlyFrm *pFly );

    // virtuelle Methoden von SwClient
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );

};

//KontactObjekt fuer die Verbindung von Formaten als Repraesentanten der
//Zeichenobjekte im StarWriter (SwClient) und den Objekten selbst im Drawing
//(SdrObjUserCall).

class SwDrawContact : public SwContact
{
    SwFrm     *pAnchor;
    SwPageFrm *pPage;
public:
    TYPEINFO();

    SwDrawContact( SwFrmFmt *pToRegisterIn, SdrObject *pObj );
    virtual ~SwDrawContact();

    const SwPageFrm *GetPage() const            { return pPage; }
          SwPageFrm *GetPage()                  { return pPage; }
    const SwFrm     *GetAnchor() const          { return pAnchor; }
          SwFrm     *GetAnchor()                { return pAnchor; }
          void       ChgAnchor( SwFrm *pNew )   { pAnchor = pNew; }
          void       ChgPage  ( SwPageFrm *pNew){ pPage   = pNew; }

          void       ChkPage();     //Muss immer nach SetAnchorPos() gerufen
                                    //werden. Das Objekt haengt sich dann an
                                    //die richtige Seite.
          SwPageFrm* FindPage( const SwRect &rRect );

    //Fuegt das SdrObject in die Arrays (SwPageFrm und SwFrm) des Layouts ein.
    //Der Anker wird Anhand des Attributes SwFmtAnchor bestimmt.
    //Das Objekt wird ggf. beim alten Anker abgemeldet.
    void ConnectToLayout( const SwFmtAnchor *pAnch = 0 );
    void DisconnectFromLayout( BOOL bRemoveFromPage = TRUE );   //Abmelden beim Anker.

    // virtuelle Methoden von SwClient
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );

    // virtuelle Methoden von SdrObjUserCall
    virtual void Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle& rOldBoundRect);

    // wird von Changed() und auch vom UndoDraw benutzt, uebernimmt
    // das Notifien von Absaetzen, die ausweichen muessen
    void _Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle* pOldBoundRect);

    //Moved alle SW-Verbindungen zu dem neuen Master.
    void ChangeMasterObject( SdrObject *pNewMaster );
};

#endif

