/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DirtyTagWrapCheck.java,v $
 * $Revision: 1.3 $
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
