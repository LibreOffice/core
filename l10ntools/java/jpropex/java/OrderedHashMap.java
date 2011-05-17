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


import java.util.*;

// LinkedHashMap implrementation
public class OrderedHashMap
{
    private HashMap hm = new HashMap();
    private LinkedList list  = new LinkedList();

    public Iterator    iterator()   { return list.iterator(); }

    public boolean isEmpty()        { return hm.isEmpty(); }
    public Object get( Object key ) { return hm.get( key ); }
    public Object get( int index )  { return hm.get( list.get( index ) ); }
    public Iterator keys()       { return list.iterator(); }
    public Object add( Object key , Object value )
    {
        list.add( key );
        return hm.put( key, value );
    }
    public Object add( int index , Object key , Object value )
    {
        list.add( index , key );
        return hm.put( key, value );
    }
    public Object remove( Object key )
    {
        list.remove( list.indexOf( key ) );
        return hm.remove( key );
    }
    public void move( int idxFrom , int idxTo )
    {
        Object key = list.get( idxFrom );
        list.remove( idxFrom );
        list.add( idxTo , key );
    }
    public void move( Object key , int idxTo )
    {
       move( list.indexOf( key ) , idxTo );
    }
    public int size()
    {
        return hm.size();
    }
    public Enumeration elements()
    {
        return new OHMenum( this );
    }
}

final class OHMenum implements Enumeration
{
    OrderedHashMap ohm;
    int index = 0;

    private OHMenum(){};
    public OHMenum( OrderedHashMap ohm ){
        this.ohm = ohm ;
    }

    public boolean hasMoreElements()
    {
        return index < ohm.size();
    }
    public Object nextElement()
    {
        return ohm.get( index++ );
    }
}
