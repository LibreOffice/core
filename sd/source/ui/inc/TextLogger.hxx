/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextLogger.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:17:26 $
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

#ifndef SD_TEXT_LOGGER_HXX
#define SD_TEXT_LOGGER_HXX

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

class String;
class VclWindowEvent;

namespace sd { namespace notes {

class EditWindow;

class TextLogger
{
public:
    static TextLogger& Instance (void);

    void AppendText (const char* sText);
    void AppendText (const String& sText);
    void AppendNumber (long int nValue);

    void ConnectToEditWindow (EditWindow* pWindow);

private:
    static TextLogger* spInstance;
    EditWindow* mpEditWindow;

    TextLogger (void);

    DECL_LINK(WindowEventHandler, VclWindowEvent*);
};

} } // end of namespace ::sd::notes

#endif
