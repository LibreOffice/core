#/usr/bin/env python

import re,os

def grep(pattern,dirname,names):
	for name in names:
		filename = os.path.join(dirname,name)
		if os.path.isfile(filename):
			lines = open(filename,"r").readlines()
			for line in lines:
				if pattern.search(line):
					print filename
					break

def find(regexp,dirName = "."):
	os.path.walk(dirName,grep,re.compile(regexp))
	

if __name__ == "__main__":
	import sys
	if len(sys.argv) == 3:
		directory = sys.argv[2]
	else:
		directory = "."

	dir(sys)
	os.path.walk(directory,grep,re.compile(sys.argv[1]))
