/*************************************************************************
 *
 *  $RCSfile: svdviter.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:01 $
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

#ifndef _SVDVITER_HXX
#define _SVDVITER_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

class OutputDevice;
class Window;
class SdrView;
class SdrPageView;
class SdrModel;
class SdrPage;
class SdrObject;
class SetOfByte;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrViewIter {
    const SdrModel*  pModel;
    const SdrPage*   pPage;
    const SdrObject* pObject;
    SdrView*   pAktView;
    FASTBOOL   bNoMasterPage;
    USHORT     nListenerNum;
    USHORT     nPageViewNum;
    USHORT     nOutDevNum;
private:
    void          ImpInitVars();
    SdrView*      ImpFindView();
    SdrPageView*  ImpFindPageView();
    OutputDevice* ImpFindOutDev();
    Window*       ImpFindWindow();
    FASTBOOL      ImpCheckPageView(SdrPageView* pPV) const;
public:
    SdrViewIter(const SdrModel*  pModel);
    SdrViewIter(const SdrPage*   pPage, FASTBOOL bNoMasterPage=FALSE);
    SdrViewIter(const SdrObject* pObject, FASTBOOL bNoMasterPage=FALSE);
    SdrView*      FirstView();
    SdrView*      NextView();
    SdrPageView*  FirstPageView();
    SdrPageView*  NextPageView();
    OutputDevice* FirstOutDev();
    OutputDevice* NextOutDev();
    Window*       FirstWindow();
    Window*       NextWindow();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _JUST_DESCRIPTION

Mit dieser Klasse kann man rausbekommen:
- SdrView* First/NextView()
  - Alle Views in denen ein Models dargestellt wird
  - Alle Views in denen eine bestimme Page sichtbar ist (ww. auch als MasterPage)
  - Alle Views in denen ein bestimmes Objekt sichtbar ist (ww. auch auf MasterPage)
- SdrPageView* First/NextPageView()
  - Alle PageViews in denen ein Models dargestellt wird
  - Alle PageViews in denen eine bestimme Page sichtbar ist (ww. auch als MasterPage)
  - Alle PageViews in denen ein bestimmes Objekt sichtbar ist (ww. auch auf MasterPage)
- OutputDevice* First/NextOutDev()
  - Alle OutputDevices in denen ein Models dargestellt wird
  - Alle OutputDevices in denen eine bestimme Page sichtbar ist (ww. auch als MasterPage)
  - Alle OutputDevices in denen ein bestimmes Objekt sichtbar ist (ww. auch auf MasterPage)
- Window* First/NextWindow()
  - Alle Windows in denen ein Models dargestellt wird
  - Alle Windows in denen eine bestimme Page sichtbar ist (auch als MasterPage)
  - Alle Windows in denen ein bestimmes Objekt sichtbar ist (auch auf MasterPage)
Ob die Auswahl auf ein(e) bestimmte(s) Page/Objekt beschraenkt wird, bestimmt man
durch die Wahl des Konstruktors.

Es werden u.a. auch berueksichtigt:
- Layer Sichtbarkeitsstatus
- Visible Layer von MasterPages
- Mehrfachlayer bei Gruppenobjekten

Es wird nicht berueksichtigt:
- Ob die Pages/Objekte wirklich schon gepaintet wurden oder noch ein Invalidate ansteht, ...
- Ob die Pages/Objekte in einem Window im sichtbaren Bereich liegen

#endif // _JUST_DESCRIPTION

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDVITER_HXX

