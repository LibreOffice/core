/*************************************************************************
 *
 *  $RCSfile: docprev.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:38 $
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

#ifndef _SD_DOCPREV_HXX_
#define _SD_DOCPREV_HXX_

#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _SV_GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#ifndef _SD_FADEDEF_H
#include <fadedef.h>
#endif

class GDIMetaFile;

class SdDocPreviewWin : public Control
{
protected:
    GDIMetaFile*    pMetaFile;
    BOOL            bInEffect;
    Link            aClickHdl;
    SfxObjectShell* m_pObj;

    virtual void    Paint( const Rectangle& rRect );
    static void     CalcSizeAndPos( GDIMetaFile* pFile, Size& rSize, Point& rPoint );
    static void     ImpPaint( GDIMetaFile* pFile, OutputDevice* pVDev );

    static const int FRAME;

public:
                    SdDocPreviewWin( Window* pParent, const ResId& rResId );
                    SdDocPreviewWin( Window* pParent );
                    ~SdDocPreviewWin() { delete pMetaFile; }
    void            SetObjectShell( SfxObjectShell* pObj, USHORT nShowPage = 0 );
    void            SetGDIFile( GDIMetaFile* pFile );
    virtual void    Resize();
    void            ShowEffect( ::com::sun::star::presentation::FadeEffect eEffect, FadeSpeed eSpeed );

    virtual long    Notify( NotifyEvent& rNEvt );

    void            SetClickHdl( const Link& rLink ) { aClickHdl = rLink; }
    const Link&     GetClickHdl() const { return aClickHdl; }

};

#endif

