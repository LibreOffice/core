/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DirtyTagWrapCheck.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-11 09:07:18 $
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
/*
 * Created on 2005
 *  by Christian Schmidt
 */
package com.sun.star.tooling.DirtyTags;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;



public class DirtyTagWrapCheck {
    static String line="";
    public static void main(String[] args) {
        try {
            File fi = new File("D:\\Testfiles\\KID_helpcontent.sdf");//Copy of
            FileReader fr = new FileReader(fi);
            BufferedReader br = new BufferedReader(fr);

             int readCounter=0;
             int missCounter=0;
             int lineErrorCounter=0;
            while((line=br.readLine())!=null){
                readCounter++;
                String [] split = line.split("\t");
                if(split.length<15){

                    lineErrorCounter++;
                    continue;
                }
                String string = split[10];
                String wrapped = DirtyTagWrapper.wrapString(string);
                String unwrapped=DirtyTagWrapper.unwrapString(wrapped);
                if(!string.equals(unwrapped)){

                    missCounter++;
                    System.out.println(""+readCounter+"\n"+string+"\n"+unwrapped+"\n"+wrapped+"\n");
                }
             }
            System.out.println("Fertig "+readCounter+" "+missCounter+" "+lineErrorCounter);
        } catch (FileNotFoundException e) {
            //
            e.printStackTrace();
        } catch (IOException e) {
            //
            e.printStackTrace();
        } catch (DirtyTagWrapper.TagWrapperException e) {
            System.out.println(e.getMessage()+"\n"+line+"\n");

        }

    }
}
