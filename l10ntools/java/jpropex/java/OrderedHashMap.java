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
