/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: graph.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:55:21 $
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

#ifndef _SV_GRAPH_H
#define _SV_GRAPH_H

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

// ---------------
// - GraphicType -
// ---------------

enum GraphicType
{
    GRAPHIC_NONE,
    GRAPHIC_BITMAP,
    GRAPHIC_GDIMETAFILE,
    GRAPHIC_DEFAULT
};

// -----------------
// - GraphicReader -
// -----------------

class ReaderData;

class VCL_DLLPUBLIC GraphicReader
{
protected:

    String          maUpperName;
    ReaderData*     mpReaderData;
    BOOL            mbIsReading;

                    GraphicReader() :
                        mpReaderData( NULL ),
                        mbIsReading( FALSE ) {}

public:

    virtual         ~GraphicReader();

    const String&   GetUpperFilterName() const { return maUpperName; }
    ReaderData*     GetReaderData() const { return mpReaderData; }
    BOOL            IsReading() const { return mbIsReading; }

    // TODO: when incompatible changes are possible again
    // the preview size hint should be redone
    BOOL            IsPreviewModeEnabled() const;
    void            DisablePreviewMode();
    void            SetPreviewSize( const Size& );
    Size            GetPreviewSize() const;
};

#endif // _SV_GRAPH_H
