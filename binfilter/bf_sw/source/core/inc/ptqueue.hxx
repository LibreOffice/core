/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _PTQUEUE_HXX
#define _PTQUEUE_HXX
namespace binfilter {

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

    static void Remove( ViewShell *pSh );
};

} //namespace binfilter
#endif
