# LibreOfficeKit tilebench joining tests

This folder contain documents to run through `tilebench --join` to detect problems. To run the tests, simply invoke:

    bin/run tilebench $PWD/instdir/program libreofficekit/qa/data/join/<filename> --join

After the tests are complete, you may see an output like this:

```
Failed 81 joins
	Zoom 0.5 bad tiles: 23 with 5637 mismatching pixels
	Zoom 0.6 bad tiles: 12 with 1341 mismatching pixels
	...
	Zoom 2 bad tiles: 3 with 1675 mismatching pixels
```

