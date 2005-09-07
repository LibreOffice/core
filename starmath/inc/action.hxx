/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: action.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 14:56:44 $
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
#ifndef ACTION_HXX
#define ACTION_HXX

#ifndef _UNDO_HXX //autogen
#include <svtools/undo.hxx>
#endif

#ifndef FORMAT_HXX
#include "format.hxx"
#endif

class SmDocShell;


class SmFormatAction: public SfxUndoAction
{
    SmDocShell  *pDoc;
    SmFormat    aOldFormat;
    SmFormat    aNewFormat;

public:
    SmFormatAction(SmDocShell *pDocSh, const SmFormat& rOldFormat, const SmFormat& rNewFormat);

    virtual void Undo();
    virtual void Redo();
    virtual void Repeat(SmDocShell *pDocSh);
    virtual String GetComment() const;
};

#endif

