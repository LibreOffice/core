/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ptqueue.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:53:28 $
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
