/*************************************************************************
 *
 *  $RCSfile: Cache.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:27:53 $
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

package com.sun.star.lib.uno.protocols.urp;


import java.util.Hashtable;


class Cache {
    /**
     * When set to true, enables various debugging output.
     */
    static private final boolean DEBUG = false;

    private static class Entry {
        short _prev;
        short _next;

        Object _content;

        Entry(short prev, short next, Object content) {
            _prev = prev;
            _next = next;
            _content = content;
        }
    }

    private Entry _entrys[];
    private short _size;
    private short _last;
    private short _first;

    private Hashtable _keyMap = new Hashtable();

    Cache(short size) {
        _size = size;
        _entrys = new Entry[size];

        for(short i = 0; i < size; ++ i) {
            _entrys[i] = new Entry((short)(i - 1), (short)(i + 1), new Object());
        }

        _first = 0;
        _last = 4;

        _entrys[_first]._prev = -1; // end of list
        _entrys[_last]._next = -1; // end of list
    }

    short add(boolean found[], Object content) {
        if(DEBUG) System.err.println("##### " + getClass().getName() +  ".add:" + content);

        short index = -1;

//          for(index = 0; index < _entrys.length && !_entrys[index]._content.equals(content); ++ index);
        Short ii = (Short)_keyMap.get(content);

        if(ii != null)
            index = ii.shortValue();


        if(index < 0 || index >= _entrys.length) { // not found
            if(DEBUG) System.err.println("##### " + getClass().getName() +  ".add - not found:" + content + " index:" + _last);

            // remove last from list
            index = _last;
            _last = _entrys[_last]._prev;
            _entrys[_last]._next = -1; // end of list

            // insert last as head
            _entrys[index]._next = _first;
            _entrys[_first]._prev = index;

            _entrys[index]._content = content;

            _first = index;
            _entrys[_first]._prev = -1; // end of list

            _keyMap.put(new Short(index), content);

            found[0] = false;
        }
        else { // found
            if(DEBUG) System.err.println("##### " + getClass().getName() +  ".add - found:" + content + " " + index);

            touch(index);

            found[0] = true;
        }

        return index;
    }

    void touch(short index) {
        if(DEBUG) System.err.println("##### " + getClass().getName() +  ".touch:" + index);

        // remove index of list
        if(index != _first && _entrys[index]._prev != -1)
            _entrys[_entrys[index]._prev]._next = _entrys[index]._next;

        if(index != _last)
            _entrys[_entrys[index]._next]._prev = _entrys[index]._prev;
        else {
            _last = _entrys[_last]._prev;
            _entrys[_last]._next = -1; // end of list
        }


        // insert index at first
        _entrys[index]._next = _first;
        _entrys[_first]._prev = index;

        _first = index;
        _entrys[_first]._prev = -1; // end of list
    }


    void list() {
        System.err.println("################# listing cache #############" + _first + " " + _last);

        short curr = _first;
        do {
            System.err.println("e:" + _entrys[curr]._content);

            curr = _entrys[curr]._next;
        }
        while(curr != -1);

        System.err.println();
    }

    static public void main(String args[]) {
        Cache cache = new Cache((short)5);

        cache.list();

        boolean found[] = new boolean[1];

        for(int i = 0; i < 7; ++ i) {
            short index = cache.add(found, new Integer(i));

            System.err.println("added: " + i + " "  + found[0]);
            cache.list();
        }

        for(int x = 0; x < 2; x++) {
            cache.add(found, new Integer(5));
            cache.list();

            cache.add(found, new Integer(4));
            cache.list();
        }
    }
}
