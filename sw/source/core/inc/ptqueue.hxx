/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
