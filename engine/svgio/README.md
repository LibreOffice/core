# SVG Reader

## Introduction
The **svgio** module is used to read **SVG** (Scalable Vector Graphics[1])
files. It is an XML based format for vector graphics.

This module contains `svgio/source/svgreader` which is used for embedding an
SVG file with "Insert -> Picture -> From File".

SVG is an open standard provided by the World Wide Web Consortium (W3C).

[1] [Scalable Vector Graphics](https://en.wikipedia.org/wiki/Scalable\_Vector\_Graphics)

## How does it work?
`svgio` module uses sax for reading xml and turns it into `drawinglayer` primitives.
The rendering is done via `drawinglayer` primitives. For more information, you should
refer to [drawinglayer](../drawinglayer) documentation.

## Known Bugs
Known remaining bugs for this module are gathered here:

* [Bug 88278 - [META] SVG import image filter (all modules)](https://bugs.documentfoundation.org/show\_bug.cgi?id=88278)

## Dependencies
Direct dependencies for **emfio** are [**drawinglayer**](../drawinglayer) and
[**sax**](../sax).

## Related Software
* [librsvg](https://en.wikipedia.org/wiki/Librsvg)
* [SVG++](http://svgpp.org/)

## References
Documentation for the SVG format is available on the W3C website:

* [SVG page at W3C](https://www.w3.org/Graphics/SVG/)
* [SVG primer](https://www.w3.org/Graphics/SVG/IG/resources/svgprimer.html)
