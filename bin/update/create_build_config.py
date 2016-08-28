#! /usr/bin/env python3

import json
import sys
import os

def main(argv):
    if len(argv) < 6:
        print("Usage: create_build_config.py $PRODUCTNAME $VERSION $BUILDID $UPDATECHANNEL $TARGETDIR")

    data = { 'productName' : argv[1],
            'version' : argv[2],
            'buildNumber' : argv[3],
            'updateChannel' : argv[4]
            }

    with open(os.path.join(argv[5], "build_config.json"), "w") as f:
        json.dump(data, f, indent=4)

if __name__ == "__main__":
    main(sys.argv)
