package org.libreoffice.ui;

import java.io.File;
import java.io.FileFilter;
import java.io.FilenameFilter;

public class FileUtilities {
	
	static final int ALL = -1;
	static final int DOC = 0;
	static final int CALC = 1;
	static final int IMPRESS = 2;
	
	private static String[] fileExtensions = {".odt",".ods",".odp"};
	
	static boolean isDoc(String filename){
		if( filename.endsWith( fileExtensions[ DOC ] ) ){
			return true;
		}
		return false;
	}
	
	static boolean isCalc(String filename){
		if( filename.endsWith( fileExtensions[ CALC ] ) ){
			return true;
		}
		return false;
	}
	
	static boolean isImpress(String filename){
		if( filename.endsWith( fileExtensions[ IMPRESS ] ) ){
			return true;
		}
		return false;
	}
	
	static FileFilter getFileFilter(int mode){
		if( mode != ALL){
			final String ext = fileExtensions[ mode ];
			return new FileFilter() {
				
				public boolean accept(File pathname) {
					if( pathname.getName().endsWith( ext ) ){
						return true;
					}
					if( pathname.isDirectory() ){
						return true;
					}
					return false;
				}
			};
		}else{//return all
			return new FileFilter() {
				
				public boolean accept(File pathname) {
					// TODO Auto-generated method stub
					return true;
				}
			};
		}
	}

	static FilenameFilter getFilenameFilter(int mode){
		if( mode != ALL){
			final String ext = fileExtensions[ mode ];
			return new FilenameFilter() {
				
				public boolean accept(File dir, String filename) {
					if( filename.endsWith( ext ) ){
						return true;
					}
					if( new File( dir , filename ).isDirectory() ){
						return true;
					}
					return false;
				}
			};
		}else{
			return new FilenameFilter() {
				
				public boolean accept(File dir, String filename) {
					return true;
				}
			}; 
		}
	}
	
}
