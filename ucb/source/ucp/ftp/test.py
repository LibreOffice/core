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


def find(pattern,directory = "."):
	os.path.walk(directory,grep,re.compile(pattern))

	
if __name__ == "__main__":
	import sys
	if len(sys.argv) == 2:
		find(sys.argv[1])
	elif len(sys.argv) == 2:
		find(sys.argv[2],sys.argv[1])
