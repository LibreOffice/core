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
