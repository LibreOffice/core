/*************************************************************************
 *
 *  $RCSfile: ptqueue.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:21 $
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
#ifndef _PTQUEUE_HXX
#define _PTQUEUE_HXX

//Leider vertragen wir es nicht so gut, wenn wir mehr als ein Paint
//gleichzeitig verarbeiten sollen. Insbesondere beim Drucken kommt dies
//leider haeufig vor.
//SwRootFrm::Paint() stellt fest, dass es zum zweitenmal gerufen wird, und
//traegt das Rechteck sowie die dazugehoerige Shell in den PaintCollector ein.
//Diejenigen stellen, die moeglicherweise das doppelte Paint "Verursachen"
//brauchen nur noch zum richtigen Zeitpunkt die gesammelten Paints verarbeiten.
//Derzeit bietet sich der Druckvorgang an, und zwar nach dem Druck von jeweils
//einer Seite.

//Das Invalidieren der Windows direkt aus dem RootFrm::Paint hat sich als nicht
//erfolgreich erwiesen, weil die Paints dann in den allermeisten Faellen
//wiederum zu einem sehr unguenstigen Zeitpunkt ankommen.
//Nach dem Druck jeder Seite ein Update auf alle Fenster auszuloesen scheint
//auch nicht angeraten, weil einerseits die edit-Windows nicht im direkten
//Zugriff sind und anderseits das notwendige Update je nach Plattform extrem
//teuer werden kann.

class SwQueuedPaint;
class ViewShell;
class SwRect;

class SwPaintQueue
{
public:
    static SwQueuedPaint *pQueue;

    static void Add( ViewShell *pSh, const SwRect &rNew );
    static void Remove( ViewShell *pSh );
    static void Repaint();
};

#endif
