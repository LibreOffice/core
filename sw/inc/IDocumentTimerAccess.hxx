/*************************************************************************
 *
 *  $RCSfile: IDocumentTimerAccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 09:37:13 $
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

 #ifndef IDOCUMENTTIMERACCESS_HXX_INCLUDED
 #define IDOCUMENTTIMERACCESS_HXX_INCLUDED

 /** Get information about the current document state
 */
 class IDocumentTimerAccess
 {
 public:
    /**
    Set modus to start, i.e. start timer if block count == 0
    */
    virtual void StartIdling() = 0;

    /**
    Set modus to stopped, i.e. stop timer if running
    */
    virtual void StopIdling() = 0;

    /**
    Increment block count, stop timer if running
    */
    virtual void BlockIdling() = 0;

    /**
    Decrement block count, start timer if block count == 0 AND modus == start
    */
    virtual void UnblockIdling() = 0;

 protected:
    virtual ~IDocumentTimerAccess() {};
 };

 #endif // IDOCUMENTTIMERACCESS_HXX_INCLUDED
