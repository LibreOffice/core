/*************************************************************************
 *
 *  $RCSfile: editsrc.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:49 $
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

#ifndef SC_EDITSRC_HXX
#define SC_EDITSRC_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"           // ScRange, ScAddress
#endif

#ifndef _SVX_UNOEDSRC_HXX
#include <svx/unoedsrc.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

class ScEditEngineDefaulter;
class SfxBroadcaster;
class SfxHint;
class SvxEditEngineForwarder;

class ScDocShell;
class ScHeaderFooterContentObj;


class ScHeaderFooterEditSource : public SvxEditSource, public SfxListener
{
private:
    ScHeaderFooterContentObj*   pContentObj;
    USHORT                      nPart;
    ScEditEngineDefaulter*      pEditEngine;
    SvxEditEngineForwarder*     pForwarder;

public:
                                ScHeaderFooterEditSource( ScHeaderFooterContentObj* pContent,
                                                            USHORT nP );
    virtual                     ~ScHeaderFooterEditSource();

    //! GetEditEngine nur als Uebergang, bis die Feld-Funktionen am Forwarder sind !!!
    ScEditEngineDefaulter*      GetEditEngine() { GetTextForwarder(); return pEditEngine; }

    virtual SvxEditSource*      Clone() const ;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};


class ScCellEditSource : public SvxEditSource, public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    ScEditEngineDefaulter*  pEditEngine;
    SvxEditEngineForwarder* pForwarder;
    BOOL                    bDataValid;
    BOOL                    bInUpdate;

public:
                                ScCellEditSource(ScDocShell* pDocSh, const ScAddress& rP);
    virtual                     ~ScCellEditSource();

    //! GetEditEngine nur als Uebergang, bis die Feld-Funktionen am Forwarder sind !!!
    ScEditEngineDefaulter*      GetEditEngine() { GetTextForwarder(); return pEditEngine; }

    virtual SvxEditSource*      Clone() const ;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};


class ScAnnotationEditSource : public SvxEditSource, public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    ScEditEngineDefaulter*  pEditEngine;
    SvxEditEngineForwarder* pForwarder;
    BOOL                    bDataValid;

public:
                                ScAnnotationEditSource(ScDocShell* pDocSh, const ScAddress& rP);
    virtual                     ~ScAnnotationEditSource();

    virtual SvxEditSource*      Clone() const ;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};


#endif

